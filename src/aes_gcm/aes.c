#include "aes.h"
#include "aes_tables.h" // Tabelas S-box, Rcon, etc.
#include <string.h>

/**
 * Expands the AES key to generate the round keys for encryption.
 *
 * This function initializes the encryption round keys in the AES context (`ctx->erk`)
 * based on the provided key. It first copies the original key into the initial positions
 * of the expanded key array, and then processes subsequent words using transformations
 * including RotWord, SubWord, and Rcon to produce the remaining round keys.
 *
 * @param ctx Pointer to the AES context containing the expanded encryption keys.
 * @param key Pointer to the original AES key.
 * @param key_len Length of the original AES key in bytes (16, 24, or 32).
 */
static void key_expansion(aes_context *ctx, const uint8_t *key, size_t key_len)
{
    uint32_t temp;

    // Primeiras palavras são a própria chave
    for (int i = 0; i < ctx->rounds + 6; i++)
    {
        ctx->erk[i] = *((uint32_t *)&key[4 * i]);
    }

    // Expansão das chaves restantes
    for (int i = (key_len / 4); i < 4 * (ctx->rounds + 1); i++)
    {
        temp = ctx->erk[i - 1];

        if (i % (key_len / 4) == 0)
        {
            // RotWord + SubWord + Rcon
            temp = (sbox[(temp >> 8) & 0xFF] << 24) |
                   (sbox[(temp >> 16) & 0xFF] << 16) |
                   (sbox[(temp >> 24) & 0xFF] << 8) |
                   (sbox[temp & 0xFF]) ^ (Rcon[i / (key_len / 4)] << 24);
        }
        else if (key_len == 32 && i % 4 == 0)
        {
            // SubWord adicional para AES-256
            temp = (sbox[(temp >> 24) & 0xFF] << 24) |
                   (sbox[(temp >> 16) & 0xFF] << 16) |
                   (sbox[(temp >> 8) & 0xFF] << 8) |
                   (sbox[temp & 0xFF]);
        }

        ctx->erk[i] = ctx->erk[i - (key_len / 4)] ^ temp;
    }
}

/**
 * @brief Applies the SubBytes transformation to the AES state.
 *
 * This function substitutes each byte in the 4x4 state array using
 * the AES S-box, which is a non-linear substitution table designed
 * to provide confusion in the cipher. The substitution is performed
 * independently for each byte in the state.
 *
 * @param state The 16-byte state array representing the AES state.
 */
static void sub_bytes(uint8_t state[16])
{
    for (int i = 0; i < 16; i++)
    {
        state[i] = sbox[state[i]];
    }
}

/**
 * Desloca as linhas do estado de acordo com a especificação do AES.
 * A primeira linha é deslocada uma posição para a esquerda, a segunda linha
 * é deslocada duas posições para a esquerda e a terceira linha é deslocada
 * três posições para a esquerda.
 * @param state Vetor de 16 bytes que representa o estado do AES.
 */
static void shift_rows(uint8_t state[16])
{
    uint8_t temp;

    // Row 1 - shift 1
    temp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = temp;

    // Row 2 - shift 2
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;

    // Row 3 - shift 3
    temp = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = state[3];
    state[3] = temp;
}

/**
 * Realiza a transforma o de colunas do AES.
 *
 * A transforma o de colunas do AES   uma opera o que transforma
 * os quatro bytes de cada coluna do estado em novos quatro bytes,
 * misturando-os linearmente com os bytes das outras colunas.
 *
 * @param state Vetor de 16 bytes que representa o estado do AES.
 */
static void mix_columns(uint8_t state[16])
{
    uint8_t tmp[16];

    for (int i = 0; i < 4; i++)
    {
        tmp[4 * i + 0] = (uint8_t)(mul2[state[4 * i + 0]] ^ mul3[state[4 * i + 1]] ^ state[4 * i + 2] ^ state[4 * i + 3]);
        tmp[4 * i + 1] = (uint8_t)(state[4 * i + 0] ^ mul2[state[4 * i + 1]] ^ mul3[state[4 * i + 2]] ^ state[4 * i + 3]);
        tmp[4 * i + 2] = (uint8_t)(state[4 * i + 0] ^ state[4 * i + 1] ^ mul2[state[4 * i + 2]] ^ mul3[state[4 * i + 3]]);
        tmp[4 * i + 3] = (uint8_t)(mul3[state[4 * i + 0]] ^ state[4 * i + 1] ^ state[4 * i + 2] ^ mul2[state[4 * i + 3]]);
    }

    memcpy(state, tmp, 16);
}

/**
 * Adiciona a chave de round ao estado do AES.
 * A chave de round   um vetor de 4 palavras de 32 bits, que   expandido
 * a partir da chave secreta pelo algoritmo de expans o de chave do AES.
 * Cada palavra da chave de round   dividida em 4 bytes e XORada com o
 * estado do AES. Essa opera o   a nica opera o no AES que depende
 * da chave.
 * @param state Vetor de 16 bytes que representa o estado do AES.
 * @param round_key Vetor de 4 palavras de 32 bits (16 bytes) que representa
 * a chave de round.
 */
static void add_round_key(uint8_t state[16], const uint32_t *round_key)
{
    for (int i = 0; i < 4; i++) // 4 colunas de 32 bits
    {
        uint32_t k = round_key[i];

        // Quebra a palavra de 32 bits em 4 bytes e faz XOR com o estado
        state[4 * i + 0] ^= (k >> 24) & 0xFF;
        state[4 * i + 1] ^= (k >> 16) & 0xFF;
        state[4 * i + 2] ^= (k >> 8) & 0xFF;
        state[4 * i + 3] ^= k & 0xFF;
    }
}

/**
 * Inicializa o contexto AES com uma chave.
 * A chave pode ter 128, 192 ou 256 bits de comprimento.
 * O n mero de rounds   calculado com base no tamanho da chave.
 * @param ctx Ponteiro para o contexto AES a ser inicializado.
 * @param key Vetor de bytes que representa a chave secreta.
 * @param key_len Comprimento da chave secreta em bytes.
 */
void aes_init(aes_context *ctx, const uint8_t *key, size_t key_len)
{
    switch (key_len)
    {
    case 16:
        ctx->rounds = 10;
        break;
    case 24:
        ctx->rounds = 12;
        break;
    case 32:
        ctx->rounds = 14;
        break;
    default:
        return; // Tamanho inválido
    }
    key_expansion(ctx, key, key_len);
}

/**
 * @brief Encrypts a single block of plaintext using the AES algorithm.
 *
 * This function performs AES encryption on a 16-byte block of plaintext,
 * transforming it into a 16-byte block of ciphertext using the specified
 * AES context. The context must be initialized with the encryption key
 * and the number of rounds, which is determined by the key length.
 *
 * @param ctx Pointer to the AES context initialized with the encryption key.
 * @param input 16-byte block of plaintext data to be encrypted.
 * @param output Buffer where the 16-byte block of encrypted ciphertext will
 *               be stored.
 */

void aes_encrypt(aes_context *ctx, const uint8_t input[16], uint8_t output[16])
{
    uint8_t state[16];
    memcpy(state, input, 16);

    // Round inicial
    add_round_key(state, &ctx->erk[0]);

    // Rounds intermediários
    for (int round = 1; round < ctx->rounds; round++)
    {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, &ctx->erk[round * 4]);
    }

    // Round final
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, &ctx->erk[ctx->rounds * 4]);

    memcpy(output, state, 16);
}

/**
 * @brief Decrypts a single block of ciphertext using the AES algorithm.
 *
 * This function performs AES decryption on a 16-byte block of ciphertext,
 * transforming it into a 16-byte block of plaintext using the specified
 * AES context. The context must be initialized with the decryption key
 * and the number of rounds, which is determined by the key length.
 *
 * @param ctx Pointer to the AES context initialized with the decryption key.
 * @param input 16-byte block of ciphertext data to be decrypted.
 * @param output Buffer where the 16-byte block of decrypted plaintext will
 *               be stored.
 */
void aes_decrypt(aes_context *ctx, const uint8_t input[16], uint8_t output[16])
{
    uint8_t temp[16];
    for (int round = ctx->rounds - 1; round >= 0; --round)
    {
        // InvShiftRows
        memcpy(temp, output, 16);
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                output[i * 4 + j] = temp[((i + 3 - j) % 4) * 4 + j];
            }
        }

        // InvSubBytes
        for (int i = 0; i < 16; ++i)
        {
            output[i] = rsbox[output[i]];
        }

        // InvMixColumns (except in the first round)
        if (round > 0)
        {
            for (int i = 0; i < 4; ++i)
            {
                uint8_t a = output[i * 4 + 0];
                uint8_t b = output[i * 4 + 1];
                uint8_t c = output[i * 4 + 2];
                uint8_t d = output[i * 4 + 3];
                output[i * 4 + 0] = (uint8_t)(0x0e * a ^ 0x0b * b ^ 0x0d * c ^ 0x09 * d);
                output[i * 4 + 1] = (uint8_t)(0x09 * a ^ 0x0e * b ^ 0x0b * c ^ 0x0d * d);
                output[i * 4 + 2] = (uint8_t)(0x0d * a ^ 0x09 * b ^ 0x0e * c ^ 0x0b * d);
                output[i * 4 + 3] = (uint8_t)(0x0b * a ^ 0x0d * b ^ 0x09 * c ^ 0x0e * d);
            }
        }

        // InvAddRoundKey
        for (int i = 0; i < 16; ++i)
        {
            output[i] ^= ctx->drk[round * 4 + (i / 4)];
        }
    }
}

/**
 * @brief Encrypts or decrypts data using AES in CTR mode.
 *
 * This function encrypts or decrypts data in CTR (Counter) mode using the
 * specified AES context. The context must be initialized with the encryption
 * or decryption key and the number of rounds, which is determined by the
 * key length.
 *
 * @param ctx Pointer to the AES context initialized with the encryption
 *            or decryption key.
 * @param iv Initialization vector (IV) for the CTR mode.
 * @param input Buffer containing the data to be encrypted or decrypted.
 * @param output Buffer where the encrypted or decrypted data will be stored.
 * @param length Length of the input data in bytes.
 */
void aes_ctr_crypt(aes_context *ctx, const uint8_t iv[16],
                   const uint8_t *input, uint8_t *output, size_t length)
{
    uint8_t counter[16];
    uint8_t encrypted_counter[16];
    size_t offset = 0;

    memcpy(counter, iv, 16);

    while (length > 0)
    {
        aes_encrypt(ctx, counter, encrypted_counter);

        size_t bytes = (length < 16) ? length : 16;
        for (size_t i = 0; i < bytes; i++)
        {
            output[offset + i] = input[offset + i] ^ encrypted_counter[i];
        }

        // Incrementa o contador (big-endian)
        for (int i = 15; i >= 0; i--)
        {
            if (++counter[i] != 0)
                break;
        }

        offset += bytes;
        length -= bytes;
    }
}
