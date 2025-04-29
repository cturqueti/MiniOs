#ifndef AES_GCM_H
#define AES_GCM_H

#include "aes.h"
#include <stdint.h>
#include <stdlib.h>

// Encriptação GCM
int aes_gcm_encrypt(aes_context *ctx,
                    const uint8_t *iv, size_t iv_len,
                    const uint8_t *aad, size_t aad_len,
                    const uint8_t *plaintext, size_t pt_len,
                    uint8_t *ciphertext,
                    uint8_t tag[16]);

// Decriptação GCM com autenticação
int aes_gcm_decrypt(aes_context *ctx,
                    const uint8_t *iv, size_t iv_len,
                    const uint8_t *aad, size_t aad_len,
                    const uint8_t *ciphertext, size_t ct_len,
                    const uint8_t *tag, size_t tag_len,
                    uint8_t *plaintext);

void process_blocks(const uint8_t *H, const uint8_t *data, size_t data_len, uint8_t x[16]);

#endif // AES_GCM_H