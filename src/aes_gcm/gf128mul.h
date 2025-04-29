#ifndef GF128MUL_H
#define GF128MUL_H

#include <stdint.h>

// Multiplicação no campo GF(2^128) usando polinômio x^128 + x^7 + x^2 + x + 1
void gf128_multiply(const uint8_t *x, const uint8_t *y, uint8_t *result);

#endif // GF128MUL_H