#include "profile.h"
#include "profile_generated.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <openssl/rand.h>
#include "debug.h"

int profile_load_from_generated(c2_profile_t *profile)
{
    if (!profile) return -1;
    memset(profile, 0, sizeof(c2_profile_t));

    profile->get_uris = GENERATED_GET_URIS;
    profile->get_uri_count = GENERATED_GET_URI_COUNT;
    profile->get_verb = GENERATED_GET_VERB;
    profile->get_headers = GENERATED_GET_HEADERS;
    profile->get_header_count = GENERATED_GET_HEADER_COUNT;
    profile->metadata_transform = GENERATED_METADATA_TRANSFORM;

    profile->post_uris = GENERATED_POST_URIS;
    profile->post_uri_count = GENERATED_POST_URI_COUNT;
    profile->post_verb = GENERATED_POST_VERB;
    profile->post_headers = GENERATED_POST_HEADERS;
    profile->post_header_count = GENERATED_POST_HEADER_COUNT;
    profile->id_transform = GENERATED_ID_TRANSFORM;
    profile->output_transform = GENERATED_OUTPUT_TRANSFORM;

    profile->server_get_transform = GENERATED_SERVER_GET_TRANSFORM;
    profile->server_post_transform = GENERATED_SERVER_POST_TRANSFORM;

    profile->user_agent = GENERATED_USER_AGENT;

    return 0;
}

int profile_validate(const c2_profile_t *profile)
{
    if (!profile) return -1;
    if (profile->get_uri_count <= 0) return -1;
    if (profile->post_uri_count <= 0) return -1;
    if (!profile->user_agent) return -1;
    return 0;
}

void profile_select_uris(c2_profile_t *profile)
{
    if (!profile) return;

    uint32_t idx;
    RAND_bytes((uint8_t*)&idx, sizeof(idx));

    if (profile->get_uri_count > 0) {
        profile->selected_get_uri = profile->get_uris[idx % profile->get_uri_count];
    }

    RAND_bytes((uint8_t*)&idx, sizeof(idx));
    if (profile->post_uri_count > 0) {
        profile->selected_post_uri = profile->post_uris[idx % profile->post_uri_count];
    }
}
