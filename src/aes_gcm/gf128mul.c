#include "gf128mul.h"
#include <string.h>

/**
 * Multiplies two 128-bit numbers in the Galois Field GF(2^128).
 *
 * This function performs multiplication of two 128-bit numbers `x` and `y`
 * in the finite field GF(2^128) using the polynomial x^128 + x^7 + x^2 + x + 1.
 * The result is stored in the `result` buffer, which must be at least 16 bytes
 * in size.
 *
 * @param x      Pointer to the first 128-bit number (16 bytes).
 * @param y      Pointer to the second 128-bit number (16 bytes).
 * @param result Pointer to the buffer where the result will be stored (16 bytes).
 */

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