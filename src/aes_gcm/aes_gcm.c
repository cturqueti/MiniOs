#include "aes_gcm.h"
#include "aes_tables.h"

//------------------------------------------------------
//                  Funções GCM
//------------------------------------------------------
static void process_blocks(const uint8_t *H, const uint8_t *data,
                           size_t data_len, uint8_t x[16]);

static void ghash_complete(const uint8_t *H,
                           const uint8_t *aad, size_t aad_len,
                           const uint8_t *ciphertext, size_t ct_len,
                           uint8_t tag[16])
{
    uint8_t x[16] = {0};
    uint8_t len_bits[16];

    // Processa AAD
    process_blocks(H, aad, aad_len, x);

    // Processa ciphertext
    process_blocks(H, ciphertext, ct_len, x);

    // Adiciona os tamanhos (AAD || ciphertext) como bloco final
    uint64_t aad_bits = aad_len * 8;
    uint64_t ct_bits = ct_len * 8;
    memset(len_bits, 0, 16);
    for (int i = 0; i < 8; i++)
    {
        len_bits[i] = (aad_bits >> (56 - i * 8)) & 0xFF;
        len_bits[8 + i] = (ct_bits >> (56 - i * 8)) & 0xFF;
    }

    for (int i = 0; i < 16; i++)
    {
        x[i] ^= len_bits[i];
    }

    gf128_multiply(x, H, tag);
}

int aes_gcm_encrypt(aes_context *ctx,
                    const uint8_t *iv, size_t iv_len,
                    const uint8_t *aad, size_t aad_len,
                    const uint8_t *plaintext, size_t pt_len,
                    uint8_t *ciphertext,
                    uint8_t tag[16])
{
    // 1. Calcular H (chave de hash)
    uint8_t H[16];
    uint8_t zero_block[16] = {0};
    aes_encrypt(ctx, zero_block, H);

    // 2. Preparar J0 (IV || 0x00000001)
    uint8_t j0[16];
    memcpy(j0, iv, 12);
    memset(j0 + 12, 0, 3);
    j0[15] = 1;

    // 3. Encriptar usando CTR
    aes_ctr_crypt(ctx, j0, plaintext, ciphertext, pt_len);

    // 4. Calcular tag
    ghash_complete(H, aad, aad_len, ciphertext, pt_len, tag);

    // 5. Encriptar o tag final
    uint8_t encrypted_j0[16];
    aes_encrypt(ctx, j0, encrypted_j0);
    for (int i = 0; i < 16; i++)
    {
        tag[i] ^= encrypted_j0[i];
    }

    return 0;
}

int aes_gcm_decrypt(aes_context *ctx,
                    const uint8_t *iv, size_t iv_len,
                    const uint8_t *aad, size_t aad_len,
                    const uint8_t *ciphertext, size_t ct_len,
                    const uint8_t *tag, size_t tag_len,
                    uint8_t *plaintext)
{
    if (iv_len != 12 || tag_len != 16)
        return -1;

    // 1. Calcular H
    uint8_t H[16];
    uint8_t zero_block[16] = {0};
    aes_encrypt(ctx, zero_block, H);

    // 2. Preparar J0
    uint8_t j0[16];
    memcpy(j0, iv, 12);
    memset(j0 + 12, 0, 3);
    j0[15] = 1;

    // 3. Calcular tag esperada
    uint8_t computed_tag[16];
    ghash_complete(H, aad, aad_len, ciphertext, ct_len, computed_tag);

    // 4. Encriptar o tag final
    uint8_t encrypted_j0[16];
    aes_encrypt(ctx, j0, encrypted_j0);
    for (int i = 0; i < 16; i++)
    {
        computed_tag[i] ^= encrypted_j0[i];
    }

    // 5. Verificar autenticação
    if (memcmp(tag, computed_tag, 16) != 0)
    {
        return -2; // Autenticação falhou
    }

    // 6. Decriptar se a autenticação passou
    aes_ctr_crypt(ctx, j0, ciphertext, plaintext, ct_len);

    return 0;
}

static void process_blocks(const uint8_t *H, const uint8_t *data, size_t data_len, uint8_t x[16])
{
    aes_context ctx;
    uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x97, 0x75, 0x46, 0x42, 0x0f, 0x18}; // Exemplo de chave AES

    aes_init(&ctx, key, 16); // Inicializa o contexto AES

    size_t blocks = data_len / 16;
    size_t remaining = data_len % 16;

    // Processa cada bloco de 16 bytes
    for (size_t i = 0; i < blocks; i++)
    {
        uint8_t block[16];
        memcpy(block, data + (i * 16), 16); // Copia o bloco de dados

        uint8_t output[16];
        aes_encrypt(&ctx, block, output); // Cifra o bloco com AES
        for (int j = 0; j < 16; j++)
        {
            x[j] ^= output[j]; // XOR com o bloco cifrado
        }
    }

    // Se houver dados restantes que não completam um bloco de 16 bytes
    if (remaining > 0)
    {
        uint8_t last_block[16] = {0};
        memcpy(last_block, data + (blocks * 16), remaining);
        uint8_t output[16];
        aes_encrypt(&ctx, last_block, output); // Cifra o último bloco com AES
        for (int j = 0; j < remaining; j++)
        {
            x[j] ^= output[j]; // XOR com o bloco cifrado
        }
    }
}

//------------------------------------------------------
//                  Funções AES
//------------------------------------------------------
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

static void sub_bytes(uint8_t state[16])
{
    for (int i = 0; i < 16; i++)
    {
        state[i] = sbox[state[i]];
    }
}

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

//------------------------------------------------------
//                  Funções GF128
//------------------------------------------------------

void gf128_multiply(const uint8_t *x, const uint8_t *y, uint8_t *result)
{
    uint8_t v[16];
    uint8_t r[16] = {0};

    memcpy(v, y, 16);

    for (int i = 0; i < 128; i++)
    {
        // Verifica bit atual de x (big-endian)
        int byte_pos = i / 8;
        int bit_pos = 7 - (i % 8);
        uint8_t bit = (x[byte_pos] >> bit_pos) & 1;

        if (bit)
        {
            for (int j = 0; j < 16; j++)
            {
                r[j] ^= v[j];
            }
        }

        // Deslocamento à esquerda de v
        uint8_t carry = v[0] & 0x80;
        for (int j = 0; j < 15; j++)
        {
            v[j] = (v[j] << 1) | ((v[j + 1] & 0x80) ? 1 : 0);
        }
        v[15] = v[15] << 1;

        // Aplica redução modular se houve overflow
        if (carry)
        {
            v[15] ^= 0xE1; // x^128 + x^7 + x^2 + x + 1
        }
    }

    memcpy(result, r, 16);
}

//------------------------------------------------------
//                  Funções Utilitários
//------------------------------------------------------
bool parse_aes_gcm_packet(const uint8_t *input, size_t input_len, AesGcmPacket *out_packet,
                          size_t iv_len, size_t tag_len)
{
    if (input_len < iv_len + tag_len)
        return false;

    out_packet->iv = input;
    out_packet->iv_len = iv_len;

    out_packet->ciphertext = input + iv_len;
    out_packet->ciphertext_len = input_len - iv_len - tag_len;

    out_packet->tag = input + input_len - tag_len;
    out_packet->tag_len = tag_len;

    return true;
}
