#ifndef AES_UTILS_H
#define AES_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Estrutura para armazenar os dados extraídos
typedef struct
{
    const uint8_t *iv;
    size_t iv_len;
    const uint8_t *ciphertext;
    size_t ciphertext_len;
    const uint8_t *tag;
    size_t tag_len;
} AesGcmPacket;

// Função para decompor um buffer contendo [IV|CIPHERTEXT|TAG]
bool parse_aes_gcm_packet(const uint8_t *input, size_t input_len, AesGcmPacket *out_packet,
                          size_t iv_len, size_t tag_len);

#endif