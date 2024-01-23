#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include <stdint.h>
#include <stdio.h>

#include "random/random.h"

// Architecture parameters
#define RADIX           64
#define LOG2RADIX       6  
typedef uint64_t        digit_t;        // Unsigned 64-bit digit
typedef uint32_t        hdigit_t;       // Unsigned 32-bit digit

#if (SEC_LEVEL == 0)
    #define NBITS_FIELD     64
    #define NBYTES_FIELD    16
    #define WORDS_FIELD     (NBITS_FIELD/RADIX)
#elif (SEC_LEVEL == 1)
    #define NBITS_FIELD     128
    #define NBYTES_FIELD    16
    #define WORDS_FIELD     (NBITS_FIELD/RADIX)
#elif (SEC_LEVEL == 2)
    #define NBITS_FIELD     192
    #define NBYTES_FIELD    24
    #define WORDS_FIELD     (NBITS_FIELD/RADIX)
#elif (SEC_LEVEL == 3)
    #define NBITS_FIELD     256
    #define NBYTES_FIELD    32
    #define WORDS_FIELD     (NBITS_FIELD/RADIX)
#else
    #error -- "Unsupported SEC_LEVEL"
#endif

#ifndef FIELD_PARAMS_H
#define FIELD_PARAMS_H
#if (NBITS_FIELD == 128)
    static const digit_t p[WORDS_FIELD] =        {0x000000000000001D, 0x8000000000000000};                        // Field order p
    static const digit_t Mont_one[WORDS_FIELD] = {0xFFFFFFFFFFFFFFE3, 0x7FFFFFFFFFFFFFFF};    // R  = 2^128 (mod p)
    static const digit_t R2[WORDS_FIELD] =       {0x0000000000000D24, 0x0000000000000000};          // R2 = (2^128)^2 (mod p)
    static const digit_t pp[WORDS_FIELD] =       {0xCB08D3DCB08D3DCB, 0xBDCB08D3DCB08D3D};          // pp = -p^(-1) mod R
#endif
#if (NBITS_FIELD == 192)
    static const digit_t p[WORDS_FIELD] = {0xFFFFFFFFFFFFFF13, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    static const digit_t Mont_one[WORDS_FIELD] = {0x00000000000000ED, 0x0000000000000000, 0x0000000000000000};
    static const digit_t R2[WORDS_FIELD] = {0x000000000000DB69, 0x0000000000000000, 0x0000000000000000};
    static const digit_t iR[WORDS_FIELD] = {0xDE83C7D4CB125C9E, 0xF1F532C497393FBA, 0x4CB125CE4FEEB7A0};
    static const digit_t pp[WORDS_FIELD] = {0xDE83C7D4CB125CE5, 0xF1F532C497393FBA, 0x4CB125CE4FEEB7A0};
#endif
#if (NBITS_FIELD == 256)
    static const digit_t p[WORDS_FIELD] = {0xFFFFFFFFFFFFFFED, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF};
    static const digit_t Mont_one[WORDS_FIELD] = {0x0000000000000026, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000};
    static const digit_t R2[WORDS_FIELD] = {0x00000000000005A4, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000};
    static const digit_t iR[WORDS_FIELD] = {0x435E50D79435E50A, 0x5E50D79435E50D79, 0x50D79435E50D7943, 0x179435E50D79435E};
    static const digit_t pp[WORDS_FIELD] = {0x86BCA1AF286BCA1B, 0xBCA1AF286BCA1AF2, 0xA1AF286BCA1AF286, 0x2F286BCA1AF286BC};
#endif
#endif /* FIELD_PARAMS_H */

// // Benchmarking
// #define WARMUP          50
// #define BENCH_LOOPS     250
// #define CLK_PER_SEC     2.9E9

typedef digit_t        f_elm_t[WORDS_FIELD];
typedef digit_t        sec_key[WORDS_FIELD];
typedef unsigned uint128_t __attribute__((mode(TI)));


#define MASK(end, start) (((-(1ULL)) >> (64 - (end - start))) << start) // Compute mask from start bit to end-1 bit


// BASIC ARITHMETIC OPERATIONS ON SINGLE WORDS

// Digit multiplication
#define MUL(a, b, hi, lo){                                                                  \
    uint128_t t0 = (uint128_t)(a) * (uint128_t)(b);                                         \
    *(hi) = (uint64_t)(t0 >> RADIX);                                                        \
    (lo) = (uint64_t)t0;                                                                    \
}

// Digit addition with carry
#define ADDC(carry, a, b, c){                                                               \
    uint128_t temp = (uint128_t)(a) + (uint128_t)(b) + (uint128_t)(carry);                    \
    (carry) = (uint64_t)(temp >> RADIX);                                                      \
    (c) = (uint64_t)temp;                                                                     \
}

// Digit subtraction with borrow
#define SUBC(borrow, a, b, c){                                                              \
    uint128_t temp2 = (uint128_t)(a) - (uint128_t)(b) - (uint128_t)(borrow);                   \
    (borrow) = (uint64_t)(temp2 >> (sizeof(uint128_t) * 8 - 1));                               \
    (c) = (uint64_t)temp2;                                                                     \
}



void to_mont(const digit_t* a, f_elm_t b);

void from_mont(const f_elm_t a, digit_t* b);

void f_from_ui(f_elm_t a, uint64_t b);

void print_f_elm_l(const f_elm_t a);

// Print a field element
void print_f_elm(const f_elm_t a);

// Print a double size mp integer
void print_mp_elm(const digit_t* a, const int nwords);

// Print out an array of nbytes bytes as hex
void print_hex(const unsigned char* a, const int nbytes);

// Copy a field element
void f_copy(const f_elm_t a, f_elm_t b);

// Correction, i.e., reduction modulo p
void f_corr(f_elm_t a);

// Generate a random field element
void f_rand(f_elm_t a);

// Addition of two field elements
void f_add(const f_elm_t a, const f_elm_t b, f_elm_t c);

// Subtraction of two field elements
void f_sub(const f_elm_t a, const f_elm_t b, f_elm_t c);

// Negation of a field element
void f_neg(const f_elm_t a, f_elm_t b);

// Multiplication of two multiprecision words (without reduction)
void mp_mul(const uint64_t* a, const uint64_t* b, uint64_t* c);

// Montgomery form reduction after multiplication
void rdc_mont(const uint64_t* a, uint64_t* c);

// Multiplication of field elements
void f_mul(const f_elm_t a, const f_elm_t b, f_elm_t c);

// Multiplicative inverse of a field element
void f_inv(f_elm_t a);

// Legendre symbol of a field element
void f_leg(const f_elm_t a, unsigned char* b);

#endif