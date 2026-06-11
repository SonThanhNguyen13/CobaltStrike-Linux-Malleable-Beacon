#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    TRANSFORM_BASE64 = 0,
    TRANSFORM_BASE64URL,
    TRANSFORM_MASK,
    TRANSFORM_NETBIOS,
    TRANSFORM_NETBIOSU,
    TRANSFORM_PREPEND,
    TRANSFORM_APPEND,
} transform_type_t;

typedef enum {
    TERMINATOR_PRINT = 0,
    TERMINATOR_HEADER,
    TERMINATOR_PARAMETER,
    TERMINATOR_URI_APPEND,
} terminator_type_t;

typedef struct {
    transform_type_t type;
    const char *arg;
} transform_step_t;

typedef struct {
    const transform_step_t *steps;
    int step_count;
    terminator_type_t terminator;
    const char *terminator_arg;
} transform_chain_t;

uint8_t* transform_encode(const uint8_t *input, size_t input_len,
                          const transform_chain_t *chain, size_t *output_len);

uint8_t* transform_decode(const uint8_t *input, size_t input_len,
                          const transform_chain_t *chain, size_t *output_len);

#endif
