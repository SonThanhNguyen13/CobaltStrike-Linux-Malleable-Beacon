#include "crypto.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rand.h>

int crypto_rsa_encrypt(const uint8_t *plaintext, size_t plaintext_len,
                       uint8_t *ciphertext, size_t *ciphertext_len) {
    
    // Load RSA public key from DER format
    const unsigned char *key_ptr = BEACON_PUBLIC_KEY;
    EVP_PKEY *pkey = d2i_PUBKEY(NULL, &key_ptr, BEACON_PUBLIC_KEY_LEN);
    
    if (!pkey) {
        fprintf(stderr, "[!] Failed to load public key\n");
        return -1;
    }
    
    // Create encryption context
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return -1;
    }
    
    // Initialize encryption
    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return -1;
    }
    
    // Set padding to PKCS1
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING);
    
    // Encrypt
    if (EVP_PKEY_encrypt(ctx, ciphertext, ciphertext_len,
                        plaintext, plaintext_len) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return -1;
    }
    
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    
    return 0;
}

int crypto_aes_encrypt(const uint8_t *key, const uint8_t *iv,
                       const uint8_t *plaintext, size_t plaintext_len,
                       uint8_t *ciphertext, size_t *ciphertext_len) {
    
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    // Initialize AES-128-CBC
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    // Disable automatic padding - manual padding required for CS compatibility
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    
    // Manually add PKCS7 padding
    size_t padding_len = 16 - (plaintext_len % 16);
    size_t padded_len = plaintext_len + padding_len;
    uint8_t *padded = malloc(padded_len);
    
    memcpy(padded, plaintext, plaintext_len);
    // PKCS7: pad with N bytes of value N
    memset(padded + plaintext_len, (unsigned char)padding_len, padding_len);
    
    int len;
    int ciphertext_len_int = 0;
    
    // Encrypt
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, padded, padded_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(padded);
        return -1;
    }
    ciphertext_len_int = len;
    
    // Finalize
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(padded);
        return -1;
    }
    ciphertext_len_int += len;
    
    *ciphertext_len = ciphertext_len_int;
    
    EVP_CIPHER_CTX_free(ctx);
    free(padded);
    return 0;
}

int crypto_aes_decrypt(const uint8_t *key, const uint8_t *iv,
                       const uint8_t *ciphertext, size_t ciphertext_len,
                       uint8_t *plaintext, size_t *plaintext_len) {
    
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    // Initialize AES-128-CBC
    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    // Disable automatic padding - we'll handle it manually
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    
    int len;
    int plaintext_len_int = 0;
    
    // Decrypt
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len_int = len;
    
    // Finalize (should not fail now with padding disabled)
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len_int += len;
    
    // Remove PKCS7 padding manually
    if (plaintext_len_int > 0) {
        unsigned char padding_value = plaintext[plaintext_len_int - 1];
        
        // Validate padding
        if (padding_value > 0 && padding_value <= 16) {
            // Check if all padding bytes have the same value
            int valid_padding = 1;
            for (int i = 0; i < padding_value; i++) {
                if (plaintext[plaintext_len_int - 1 - i] != padding_value) {
                    valid_padding = 0;
                    break;
                }
            }
            
            if (valid_padding) {
                plaintext_len_int -= padding_value;
            }
        }
    }
    
    *plaintext_len = plaintext_len_int;
    
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

char* base64_encode(const uint8_t *data, size_t len) {
    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data, len);
    BIO_flush(bio);
    
    BIO_get_mem_ptr(bio, &buffer_ptr);
    
    char *encoded = malloc(buffer_ptr->length + 1);
    memcpy(encoded, buffer_ptr->data, buffer_ptr->length);
    encoded[buffer_ptr->length] = '\0';
    
    BIO_free_all(bio);
    
    return encoded;
}

uint8_t* base64_decode(const char *data, size_t *out_len) {
    BIO *bio, *b64;
    size_t len = strlen(data);
    uint8_t *decoded = malloc(len);
    
    bio = BIO_new_mem_buf(data, len);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    *out_len = BIO_read(bio, decoded, len);
    
    BIO_free_all(bio);
    
    return decoded;
}

void crypto_random_bytes(uint8_t *buffer, size_t len) {
    RAND_bytes(buffer, len);
}

char* base64url_encode(const uint8_t *data, size_t len) {
    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data, len);
    BIO_flush(bio);
    
    BIO_get_mem_ptr(bio, &buffer_ptr);
    
    char *encoded = malloc(buffer_ptr->length + 1);
    memcpy(encoded, buffer_ptr->data, buffer_ptr->length);
    encoded[buffer_ptr->length] = '\0';
    
    for (size_t i = 0; i < buffer_ptr->length; i++) {
        if (encoded[i] == '+') encoded[i] = '-';
        else if (encoded[i] == '/') encoded[i] = '_';
    }
    
    size_t out_len = buffer_ptr->length;
    while (out_len > 0 && encoded[out_len - 1] == '=') {
        encoded[out_len - 1] = '\0';
        out_len--;
    }
    
    BIO_free_all(bio);
    return encoded;
}

uint8_t* base64url_decode(const char *data, size_t *out_len) {
    size_t len = strlen(data);
    char *padded = malloc(len + 4);
    if (!padded) return NULL;
    memcpy(padded, data, len);
    
    for (size_t i = 0; i < len; i++) {
        if (padded[i] == '-') padded[i] = '+';
        else if (padded[i] == '_') padded[i] = '/';
    }
    
    while (len % 4 != 0) {
        padded[len++] = '=';
    }
    padded[len] = '\0';
    
    BIO *bio, *b64;
    uint8_t *decoded = malloc(len);
    
    bio = BIO_new_mem_buf(padded, len);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    *out_len = BIO_read(bio, decoded, len);
    
    BIO_free_all(bio);
    free(padded);
    
    return decoded;
}
