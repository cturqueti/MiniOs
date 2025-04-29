#ifndef AES_H
#define AES_H

#include <stdint.h>
#include <stdlib.h>

#define AES_BLOCK_SIZE 16

// Contexto AES contendo chaves de expansão e configuração
typedef struct
{
    uint32_t erk[64]; // Chave de encriptação expandida
    uint32_t drk[64]; // Chave de decriptação expandida
    int rounds;       // Número de rounds (10, 12 ou 14)
} aes_context;

// Inicializa o contexto AES com uma chave
void aes_init(aes_context *ctx, const uint8_t *key, size_t key_len);

// Funções básicas de cifração/decifração
void aes_encrypt(aes_context *ctx, const uint8_t input[16], uint8_t output[16]);
void aes_decrypt(aes_context *ctx, const uint8_t input[16], uint8_t output[16]);

// Modo CTR (usado pelo GCM)
void aes_ctr_crypt(aes_context *ctx, const uint8_t iv[16],
                   const uint8_t *input, uint8_t *output, size_t length);

#endif // AES_H