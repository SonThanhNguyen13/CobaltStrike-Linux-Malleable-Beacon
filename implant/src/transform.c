#include "transform.h"
#include "crypto.h"
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>

static uint8_t* transform_step_apply(const uint8_t *input, size_t input_len,
                                     const transform_step_t *step, size_t *output_len)
{
    switch (step->type) {
    case TRANSFORM_BASE64: {
        char *encoded = base64_encode(input, input_len);
        if (!encoded) { *output_len = 0; return NULL; }
        size_t len = strlen(encoded);
        uint8_t *result = malloc(len);
        if (!result) { free(encoded); *output_len = 0; return NULL; }
        memcpy(result, encoded, len);
        free(encoded);
        *output_len = len;
        return result;
    }
    case TRANSFORM_BASE64URL: {
        char *encoded = base64url_encode(input, input_len);
        if (!encoded) { *output_len = 0; return NULL; }
        size_t len = strlen(encoded);
        uint8_t *result = malloc(len);
        if (!result) { free(encoded); *output_len = 0; return NULL; }
        memcpy(result, encoded, len);
        free(encoded);
        *output_len = len;
        return result;
    }
    case TRANSFORM_MASK: {
        uint8_t *output = malloc(input_len + 4);
        if (!output) { *output_len = 0; return NULL; }
        uint32_t key;
        if (RAND_bytes((uint8_t*)&key, sizeof(key)) != 1) {
            free(output);
            *output_len = 0;
            return NULL;
        }
        memcpy(output, &key, 4);
        for (size_t i = 0; i < input_len; i++) {
            output[i + 4] = input[i] ^ ((uint8_t*)&key)[i % 4];
        }
        *output_len = input_len + 4;
        return output;
    }
    case TRANSFORM_NETBIOS: {
        size_t out_size = input_len * 2;
        uint8_t *output = malloc(out_size);
        if (!output) { *output_len = 0; return NULL; }
        for (size_t i = 0; i < input_len; i++) {
            uint8_t b = input[i];
            output[i * 2]     = 'a' + ((b >> 4) & 0x0F);
            output[i * 2 + 1] = 'a' + (b & 0x0F);
        }
        *output_len = out_size;
        return output;
    }
    case TRANSFORM_NETBIOSU: {
        size_t out_size = input_len * 2;
        uint8_t *output = malloc(out_size);
        if (!output) { *output_len = 0; return NULL; }
        for (size_t i = 0; i < input_len; i++) {
            uint8_t b = input[i];
            output[i * 2]     = 'A' + ((b >> 4) & 0x0F);
            output[i * 2 + 1] = 'A' + (b & 0x0F);
        }
        *output_len = out_size;
        return output;
    }
    case TRANSFORM_PREPEND: {
        if (!step->arg) { *output_len = 0; return NULL; }
        size_t arg_len = strlen(step->arg);
        uint8_t *output = malloc(arg_len + input_len);
        if (!output) { *output_len = 0; return NULL; }
        memcpy(output, step->arg, arg_len);
        memcpy(output + arg_len, input, input_len);
        *output_len = arg_len + input_len;
        return output;
    }
    case TRANSFORM_APPEND: {
        if (!step->arg) { *output_len = 0; return NULL; }
        size_t arg_len = strlen(step->arg);
        uint8_t *output = malloc(input_len + arg_len);
        if (!output) { *output_len = 0; return NULL; }
        memcpy(output, input, input_len);
        memcpy(output + input_len, step->arg, arg_len);
        *output_len = input_len + arg_len;
        return output;
    }
    default:
        *output_len = 0;
        return NULL;
    }
}

static uint8_t* transform_step_reverse(const uint8_t *input, size_t input_len,
                                       const transform_step_t *step, size_t *output_len)
{
    switch (step->type) {
    case TRANSFORM_BASE64: {
        uint8_t *decoded = base64_decode((const char*)input, output_len);
        if (!decoded) { *output_len = 0; return NULL; }
        return decoded;
    }
    case TRANSFORM_BASE64URL: {
        uint8_t *decoded = base64url_decode((const char*)input, output_len);
        if (!decoded) { *output_len = 0; return NULL; }
        return decoded;
    }
    case TRANSFORM_MASK: {
        if (input_len < 4) { *output_len = 0; return NULL; }
        uint32_t key;
        memcpy(&key, input, 4);
        size_t data_len = input_len - 4;
        uint8_t *output = malloc(data_len);
        if (!output) { *output_len = 0; return NULL; }
        for (size_t i = 0; i < data_len; i++) {
            output[i] = input[i + 4] ^ ((uint8_t*)&key)[i % 4];
        }
        *output_len = data_len;
        return output;
    }
    case TRANSFORM_NETBIOS: {
        if (input_len % 2 != 0) { *output_len = 0; return NULL; }
        size_t out_size = input_len / 2;
        uint8_t *output = malloc(out_size);
        if (!output) { *output_len = 0; return NULL; }
        for (size_t i = 0; i < out_size; i++) {
            uint8_t hi = input[i * 2] - 'a';
            uint8_t lo = input[i * 2 + 1] - 'a';
            output[i] = (hi << 4) | lo;
        }
        *output_len = out_size;
        return output;
    }
    case TRANSFORM_NETBIOSU: {
        if (input_len % 2 != 0) { *output_len = 0; return NULL; }
        size_t out_size = input_len / 2;
        uint8_t *output = malloc(out_size);
        if (!output) { *output_len = 0; return NULL; }
        for (size_t i = 0; i < out_size; i++) {
            uint8_t hi = input[i * 2] - 'A';
            uint8_t lo = input[i * 2 + 1] - 'A';
            output[i] = (hi << 4) | lo;
        }
        *output_len = out_size;
        return output;
    }
    case TRANSFORM_PREPEND: {
        if (!step->arg) { *output_len = 0; return NULL; }
        size_t arg_len = strlen(step->arg);
        if (input_len < arg_len) { *output_len = 0; return NULL; }
        if (memcmp(input, step->arg, arg_len) != 0) { *output_len = 0; return NULL; }
        size_t remaining = input_len - arg_len;
        uint8_t *output = malloc(remaining);
        if (!output) { *output_len = 0; return NULL; }
        memcpy(output, input + arg_len, remaining);
        *output_len = remaining;
        return output;
    }
    case TRANSFORM_APPEND: {
        if (!step->arg) { *output_len = 0; return NULL; }
        size_t arg_len = strlen(step->arg);
        if (input_len < arg_len) { *output_len = 0; return NULL; }
        if (memcmp(input + input_len - arg_len, step->arg, arg_len) != 0) { *output_len = 0; return NULL; }
        size_t remaining = input_len - arg_len;
        uint8_t *output = malloc(remaining);
        if (!output) { *output_len = 0; return NULL; }
        memcpy(output, input, remaining);
        *output_len = remaining;
        return output;
    }
    default:
        *output_len = 0;
        return NULL;
    }
}

uint8_t* transform_encode(const uint8_t *input, size_t input_len,
                          const transform_chain_t *chain, size_t *output_len)
{
    if (!input || !chain || !output_len) {
        if (output_len) *output_len = 0;
        return NULL;
    }

    uint8_t *current = malloc(input_len);
    if (!current) { *output_len = 0; return NULL; }
    memcpy(current, input, input_len);
    size_t current_len = input_len;

    for (int i = 0; i < chain->step_count; i++) {
        uint8_t *next = transform_step_apply(current, current_len, &chain->steps[i], &current_len);
        free(current);
        if (!next) {
            *output_len = 0;
            return NULL;
        }
        current = next;
    }

    *output_len = current_len;
    return current;
}

uint8_t* transform_decode(const uint8_t *input, size_t input_len,
                          const transform_chain_t *chain, size_t *output_len)
{
    if (!input || !chain || !output_len) {
        if (output_len) *output_len = 0;
        return NULL;
    }

    uint8_t *current = malloc(input_len);
    if (!current) { *output_len = 0; return NULL; }
    memcpy(current, input, input_len);
    size_t current_len = input_len;

    for (int i = chain->step_count - 1; i >= 0; i--) {
        uint8_t *next = transform_step_reverse(current, current_len, &chain->steps[i], &current_len);
        free(current);
        if (!next) {
            *output_len = 0;
            return NULL;
        }
        current = next;
    }

    *output_len = current_len;
    return current;
}
