#include "aes_gcm.h"
#include "gf128mul.h"
#include <string.h>

/**
 * @brief Completes GHASH calculation for AES-GCM.
 *
 * This function completes the GHASH calculation by adding the lengths of the
 * AAD and ciphertext to the hash value, and then multiplying the result by the
 * hash key H.
 *
 * @param H       Pointer to the 128-bit hash key H.
 * @param aad     Pointer to the additional authenticated data (AAD) to be
 *                included in the authentication process but not encrypted.
 * @param aad_len Length of the AAD in bytes.
 * @param ciphertext Pointer to the ciphertext data to be decrypted.
 * @param ct_len   Length of the ciphertext in bytes.
 * @param tag      Output buffer where the authentication tag will be stored.
 *                 Must be at least 16 bytes in size.
 */
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

/**
 * @brief Encrypts data using AES-GCM mode.
 *
 * This function performs AES encryption in Galois/Counter Mode (GCM), which
 * provides both confidentiality and authentication. It encrypts the given
 * plaintext and computes an authentication tag.
 *
 * @param ctx AES context initialized with the encryption key.
 * @param iv Initialization vector (IV) for the GCM mode.
 * @param iv_len Length of the IV in bytes. Should be 12 for GCM.
 * @param aad Additional authenticated data (AAD) to be included in the
 *            authentication process but not encrypted.
 * @param aad_len Length of the AAD in bytes.
 * @param plaintext The plaintext data to be encrypted.
 * @param pt_len Length of the plaintext in bytes.
 * @param ciphertext Output buffer where the encrypted data will be stored.
 *                   Must be at least as large as pt_len.
 * @param tag Output buffer to store the computed authentication tag.
 *            Must be 16 bytes in length.
 *
 * @return 0 on successful encryption, or a non-zero error code on failure.
 */
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

/**
 * @brief Decrypts data using AES-GCM mode.
 *
 * This function performs AES decryption in Galois/Counter Mode (GCM),
 * which provides both confidentiality and authentication. It decrypts the
 * given ciphertext and verifies the authentication tag.
 *
 * @param ctx AES context initialized with the decryption key.
 * @param iv Initialization vector (IV) for the GCM mode.
 * @param iv_len Length of the IV in bytes. Should be 12 for GCM.
 * @param aad Additional authenticated data (AAD) to be included in the
 *            authentication process but not decrypted.
 * @param aad_len Length of the AAD in bytes.
 * @param ciphertext The ciphertext data to be decrypted.
 * @param ct_len Length of the ciphertext in bytes.
 * @param tag The authentication tag to be verified.
 * @param tag_len Length of the tag in bytes. Should be 16 for GCM.
 * @param plaintext Output buffer where the decrypted data will be stored.
 *                   Must be at least as large as ct_len.
 *
 * @return 0 on successful decryption and authentication, -1 if the IV or
 *         tag length is invalid, -2 if the authentication fails.
 */
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

void process_blocks(const uint8_t *H, const uint8_t *data, size_t data_len, uint8_t x[16])
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