#include "url_builder.h"
#include <stdio.h>
#include <string.h>

void url_builder_init(url_builder_t *builder, const char *base_url)
{
    if (!builder || !base_url) return;
    size_t len = strlen(base_url);
    if (len >= URL_BUILDER_MAX) len = URL_BUILDER_MAX - 1;
    memcpy(builder->buffer, base_url, len);
    builder->buffer[len] = '\0';
    builder->len = len;
    builder->has_query = (strchr(base_url, '?') != NULL);
}

void url_builder_add_param(url_builder_t *builder, const char *key, const char *value)
{
    if (!builder || !key || !value) return;

    size_t key_len = strlen(key);
    size_t val_len = strlen(value);
    size_t needed = builder->len + 1 + key_len + 1 + val_len;

    if (needed >= URL_BUILDER_MAX) return;

    char sep = builder->has_query ? '&' : '?';
    builder->buffer[builder->len++] = sep;
    memcpy(builder->buffer + builder->len, key, key_len);
    builder->len += key_len;
    builder->buffer[builder->len++] = '=';
    memcpy(builder->buffer + builder->len, value, val_len);
    builder->len += val_len;
    builder->buffer[builder->len] = '\0';
    builder->has_query = true;
}

void url_builder_append_path(url_builder_t *builder, const char *segment)
{
    if (!builder || !segment) return;

    size_t seg_len = strlen(segment);
    size_t needed = builder->len + seg_len;

    if (needed >= URL_BUILDER_MAX) return;

    memcpy(builder->buffer + builder->len, segment, seg_len);
    builder->len += seg_len;
    builder->buffer[builder->len] = '\0';
}

const char* url_builder_get(const url_builder_t *builder)
{
    if (!builder) return NULL;
    return builder->buffer;
}
