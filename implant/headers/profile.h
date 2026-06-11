#ifndef PROFILE_H
#define PROFILE_H

#include <stdint.h>
#include <stddef.h>
#include "transform.h"

typedef struct {
    const char *name;
    const char *value;
} profile_header_t;

typedef struct {
    const char **get_uris;
    int get_uri_count;
    const char *get_verb;
    const profile_header_t *get_headers;
    int get_header_count;
    transform_chain_t metadata_transform;

    const char **post_uris;
    int post_uri_count;
    const char *post_verb;
    const profile_header_t *post_headers;
    int post_header_count;
    transform_chain_t id_transform;
    transform_chain_t output_transform;

    transform_chain_t server_get_transform;
    transform_chain_t server_post_transform;

    const char *user_agent;
    const char *selected_get_uri;
    const char *selected_post_uri;
} c2_profile_t;

int profile_load_from_generated(c2_profile_t *profile);
int profile_validate(const c2_profile_t *profile);
void profile_select_uris(c2_profile_t *profile);

#endif
