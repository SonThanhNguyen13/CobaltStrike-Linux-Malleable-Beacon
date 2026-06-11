#ifndef URL_BUILDER_H
#define URL_BUILDER_H

#include <stddef.h>
#include <stdbool.h>

#define URL_BUILDER_MAX 4096

typedef struct {
    char buffer[URL_BUILDER_MAX];
    size_t len;
    bool has_query;
} url_builder_t;

void url_builder_init(url_builder_t *builder, const char *base_url);
void url_builder_add_param(url_builder_t *builder, const char *key, const char *value);
void url_builder_append_path(url_builder_t *builder, const char *segment);
const char* url_builder_get(const url_builder_t *builder);

#endif
