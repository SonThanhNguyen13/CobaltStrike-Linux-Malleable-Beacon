#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include <stddef.h>

// RSA encryption
int crypto_rsa_encrypt(const uint8_t *plaintext, size_t plaintext_len,
                       uint8_t *ciphertext, size_t *ciphertext_len);

// AES encryption/decryption
int crypto_aes_encrypt(const uint8_t *key, const uint8_t *iv,
                       const uint8_t *plaintext, size_t plaintext_len,
                       uint8_t *ciphertext, size_t *ciphertext_len);

int crypto_aes_decrypt(const uint8_t *key, const uint8_t *iv,
                       const uint8_t *ciphertext, size_t ciphertext_len,
                       uint8_t *plaintext, size_t *plaintext_len);

// Base64 encoding (for Cookie header)
char* base64_encode(const uint8_t *data, size_t len);
uint8_t* base64_decode(const char *data, size_t *out_len);

// Random data generation
void crypto_random_bytes(uint8_t *buffer, size_t len);

char* base64url_encode(const uint8_t *data, size_t len);
uint8_t* base64url_decode(const char *data, size_t *out_len);

#endif // CRYPTO_H
