#ifndef AES_GCM_H
#define AES_GCM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AES_BLOCK_SIZE 16

typedef struct
{
    uint32_t erk[64]; // Chave de encriptação expandida
    uint32_t drk[64]; // Chave de decriptação expandida
    int rounds;       // Número de rounds (10, 12 ou 14)
} aes_context;

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

//------------------------------------------------------
//                  Funções GCM
//------------------------------------------------------

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

//------------------------------------------------------
//                  Funções AES
//------------------------------------------------------
// Inicializa o contexto AES com uma chave
void aes_init(aes_context *ctx, const uint8_t *key, size_t key_len);

// Funções básicas de cifração/decifração
void aes_encrypt(aes_context *ctx, const uint8_t input[16], uint8_t output[16]);
void aes_decrypt(aes_context *ctx, const uint8_t input[16], uint8_t output[16]);

// Modo CTR (usado pelo GCM)
void aes_ctr_crypt(aes_context *ctx, const uint8_t iv[16],
                   const uint8_t *input, uint8_t *output, size_t length);

//------------------------------------------------------
//                  Funções GF128
//------------------------------------------------------
void gf128_multiply(const uint8_t *x, const uint8_t *y, uint8_t *result);

//------------------------------------------------------
//                  Funções Utilitários
//------------------------------------------------------
// Função para decompor um buffer contendo [IV|CIPHERTEXT|TAG]
bool parse_aes_gcm_packet(const uint8_t *input, size_t input_len, AesGcmPacket *out_packet,
                          size_t iv_len, size_t tag_len);

#endif // AES_GCM_H