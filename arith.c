#include "arith.h"
#include "random/random.h"

// Field definitions
// const digit_t p[WORDS_FIELD] = {0xFFFFFFFFFFFFFF61, 0xFFFFFFFFFFFFFFFF};           // Field order p
// const digit_t Mont_one[WORDS_FIELD] = {0x000000000000009F, 0x0000000000000000};    // R  = 2^128 (mod p)
// const digit_t R2[WORDS_FIELD] = {0x00000000000062C1, 0x0000000000000000};          // R2 = (2^128)^2 (mod p)
// const digit_t pp[WORDS_FIELD] = {0xB5EFE63D2EB11B5F, 0xB11B5EFE63D2EB11};          // pp = -p^(-1) mod R

void to_mont(const digit_t *a, f_elm_t b)
{
    f_mul(a, R2, b);
}

// Convert a number from Montgomery form into value (aR -> a)
void from_mont(const f_elm_t a, digit_t *b)
{
    digit_t t0[2 * WORDS_FIELD] = {0};
    f_copy(a, t0);
    rdc_mont(t0, b);
}

void f_from_ui(f_elm_t a, uint64_t b)
{
    a[0] = b;
    for (int i = 1; i < WORDS_FIELD; i++)
    {
        a[i] = 0;
    }
}

// Print a field element
void print_f_elm_l(const f_elm_t a)
{
    printf("0x");
    for (int i = sizeof(f_elm_t) - 1; i >= 0; i--)
        printf("%02X", ((uint8_t *)(a))[i]);
    printf(" ");
}

// Print a field element
void print_f_elm(const f_elm_t a)
{
    printf("0x");
    for (int i = sizeof(f_elm_t) - 1; i >= 0; i--)
        printf("%02X", ((uint8_t *)(a))[i]);
    printf("\n");
}

// Print a double size mp integer
void print_mp_elm(const digit_t *a, const int nwords)
{
    printf("0x");
    for (int i = nwords * sizeof(f_elm_t) - 1; i >= 0; i--)
        printf("%02X", ((uint8_t *)(a))[i]);
    printf("\n");
}

// Print out an array of nbytes bytes as hex
void print_hex(const unsigned char *a, const int nbytes)
{
    printf("0x");
    for (int i = 0; i < nbytes; i++)
        printf("%02X", ((uint8_t *)(a))[i]);
    printf("\n");
}

// Copy a field element
void f_copy(const f_elm_t a, f_elm_t b)
{
    for (int i = 0; i < WORDS_FIELD; i++)
        b[i] = a[i];
}

// Correction, i.e., reduction modulo p
void f_corr(f_elm_t a)
{
    uint64_t mask, borrow = 0, carry = 0;
    for (int i = 0; i < WORDS_FIELD; i++)
    {
        SUBC(borrow, a[i], p[i], a[i]);
    }

    mask = 0 - borrow;

    for (int i = 0; i < WORDS_FIELD; i++)
        ADDC(carry, a[i], p[i] & mask, a[i]);
}

// Generate a random field element
void f_rand(f_elm_t a)
{
    randombytes((unsigned char *)a, sizeof(uint64_t) * WORDS_FIELD);
    f_corr(a); // Not uniformly random, can use rejection sampling to fix, thought it's pretty close to uniform
}

// Addition of two field elements
void f_add(const f_elm_t a, const f_elm_t b, f_elm_t c)
{
    uint64_t mask, carry = 0;

    for (int i = 0; i < WORDS_FIELD; i++)
        ADDC(carry, a[i], b[i], c[i]);

    mask = 0 - carry;
    carry = 0;
    for (int i = 0; i < WORDS_FIELD; i++)
        ADDC(carry, c[i], Mont_one[i] & mask, c[i]);

    f_corr(c);
}

// Subtraction of two field elements
void f_sub(const f_elm_t a, const f_elm_t b, f_elm_t c)
{
    uint64_t mask, borrow = 0, carry = 0;

    for (int i = 0; i < WORDS_FIELD; i++)
        SUBC(borrow, a[i], b[i], c[i]);

    mask = 0 - borrow;

    for (int i = 0; i < WORDS_FIELD; i++)
        SUBC(carry, c[i], Mont_one[i] & mask, c[i])

    f_corr(c);
}

// Negation of a field element
void f_neg(const f_elm_t a, f_elm_t b)
{
    uint64_t borrow = 0;

    for (int i = 0; i < WORDS_FIELD; i++)
        SUBC(borrow, p[i], a[i], b[i]);

    f_corr(b);
}

// Multiplication of two multiprecision words (without reduction)
void mp_mul(const uint64_t *a, const uint64_t *b, uint64_t *c)
{ // Schoolbook multiplication
    uint64_t carry, UV[2], t = 0, u = 0, v = 0;

    for (int i = 0; i < WORDS_FIELD; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            carry = 0;
            MUL(a[j], b[i - j], UV + 1, UV[0]);
            ADDC(carry, UV[0], v, v);
            ADDC(carry, UV[1], u, u);
            t += carry;
        }
        c[i] = v;
        v = u;
        u = t;
        t = 0;
    }

    for (int i = WORDS_FIELD; i < 2 * WORDS_FIELD - 1; i++)
    {
        for (int j = i - WORDS_FIELD + 1; j < WORDS_FIELD; j++)
        {
            carry = 0;
            MUL(a[j], b[i - j], UV + 1, UV[0]);
            ADDC(carry, UV[0], v, v);
            ADDC(carry, UV[1], u, u);
            t += carry;
        }
        c[i] = v;
        v = u;
        u = t;
        t = 0;
    }
    c[2 * WORDS_FIELD - 1] = v;
}

// Montgomery form reduction after multiplication
void rdc_mont(const uint64_t *a, uint64_t *c)
{
    // c = a*R^-1 mod p, where R = 2^256.
    // If a < 2^256*p, the output c is in the range [0, p).
    // a is assumed to be in Montgomery representation.
    uint64_t mask, carry = 0;
    uint64_t t0[2 * WORDS_FIELD], t1[WORDS_FIELD];

    for (int i = 0; i < WORDS_FIELD; i++)
        c[i] = a[i];

    mp_mul(c, pp, t0);
    f_copy(t0, t1);
    mp_mul(t1, p, t0);

    for (int i = 0; i < 2 * WORDS_FIELD; i++)
        ADDC(carry, t0[i], a[i], t0[i]);

    f_copy((uint64_t *)&t0[WORDS_FIELD], c);
    f_corr(c);

    mask = 0 - carry;
    carry = 0;
    for (int i = 0; i < WORDS_FIELD; i++)
        ADDC(carry, c[i], Mont_one[i] & mask, c[i]);

    f_corr(c);
}

void f_mul(const f_elm_t a, const f_elm_t b, f_elm_t c)
{
    uint64_t t0[2 * WORDS_FIELD] = {0};

    mp_mul(a, b, t0);
    rdc_mont(t0, c);
}

void f_inv(f_elm_t a)
{
#if (NBITS_FIELD == 128)
    f_elm_t t0;
    unsigned int i;

    f_copy(Mont_one, t0);

    /* p - 2 =  1000000000000000\
                0000000000000000\
                0000000000000000\
                0000000000000000\
                0000000000000000\
                0000000000000000\
                0000000000000000\
                0000000000011011\
    */

    // bit = 127
    f_mul(t0, a, t0);

    // bits 126 down to 5
    for (i = 126; i > 4; i--)
    {
        f_mul(t0, t0, t0);
        // f_mul(t0, a, t0);
    }

    // bit = 4
    f_mul(t0, t0, t0);
    f_mul(t0, a, t0);

    // bit = 3
    f_mul(t0, t0, t0);
    f_mul(t0, a, t0);

    // bit = 2
    f_mul(t0, t0, t0);

    // bit = 1
    f_mul(t0, t0, t0);
    f_mul(t0, a, t0);

    // bit = 0
    f_mul(t0, t0, t0);
    f_mul(t0, a, t0);

    f_copy(t0, a);
#endif

#if (NBITS_FIELD == 192)
     f_elm_t t[5];
    unsigned int i, j;

    f_copy(a, t[0]);
    f_copy(a, t[1]);

    /* p - 2 =     0b   11111111 11111111
                        11111111 11111111
                        11111111 11111111
                        11111111 11111111

                        11111111 11111111
                        11111111 11111111
                        11111111 11111111
                        11111111 11111111

                        11111111 11111111
                        11111111 11111111

                        11111111 11111111

                        11111111 00010001
    */

    // First 128 bits = 2^7 bits
    for(j = 0; j < 7; j++){
        for (i = 0; i < (1 << j); i++)
            f_mul(t[0], t[0], t[0]);
        f_mul(t[0], t[1], t[0]);
        if(j == 2) f_copy(t[0], t[2]);  // t[2] = a^(2^16-1) = a^0b 11111111
        if(j == 3) f_copy(t[0], t[3]);  // t[3] = a^(2^16-1) = a^0b 11111111 11111111
        if(j == 4) f_copy(t[0], t[4]);  // t[4] = a^(2^16-1) = a^0b 11111111 11111111 11111111 11111111
        f_copy(t[0], t[1]);             // = a^(2^(2^(j+1)) - 1)
    }
    
    /* t[3] = a ^ 0b 11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
    */

    // Next 32 bits
    for (i = 0; i < 32; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[4], t[0]);

    // Next 16 bits
    for (i = 0; i < 16; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[3], t[0]);

    // Next 8 bits
    for (i = 0; i < 8; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[2], t[0]);

    // Next 8 bits
    // bit = 7
    f_mul(t[0], t[0], t[0]);
    // bit = 6
    f_mul(t[0], t[0], t[0]);
    // bit = 5
    f_mul(t[0], t[0], t[0]);
    // bit = 4
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);
    // bit = 3
    f_mul(t[0], t[0], t[0]);
    // bit = 2
    f_mul(t[0], t[0], t[0]);
    // bit = 1
    f_mul(t[0], t[0], t[0]);
    // bit = 0
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);

    f_copy(t[0], a);
#endif

#if (NBITS_FIELD == 256)
    f_elm_t t[6];
    unsigned int i, j;

    f_copy(a, t[0]);
    f_copy(a, t[1]);

    /* p - 2 =     0b   0 1111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 1 1101011\
    */

    // Compute a^(p-2)
    // bit 255 = 0

    // First 128 bits = 2^7 bits
    for(j = 0; j < 7; j++){
        for (i = 0; i < (1 << j); i++)
            f_mul(t[0], t[0], t[0]);
        f_mul(t[0], t[1], t[0]);
        if(j == 2) f_copy(t[0], t[2]);  // t[2] = a^(2^8  - 1) = a^0b 11111111
        if(j == 3) f_copy(t[0], t[3]);  // t[3] = a^(2^16 - 1) = a^0b 11111111 11111111
        if(j == 4) f_copy(t[0], t[4]);  // t[4] = a^(2^32 - 1) = a^0b 11111111 11111111 11111111 11111111
        if(j == 5) f_copy(t[0], t[5]);  // t[5] = a^(2^64 - 1) = a^0b 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
        f_copy(t[0], t[1]);             // = a^(2^(2^(j+1)) - 1)
    }
    
    /* t[3] = a ^ 0b 11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
    */

    // Next 64 bits
    for (i = 0; i < 64; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[5], t[0]);

    // Next 32 bits
    for (i = 0; i < 32; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[4], t[0]);

    // Next 16 bits
    for (i = 0; i < 16; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[3], t[0]);

    // Next 8 bits
    for (i = 0; i < 8; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[2], t[0]);

    // bit = 6
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);
    // bit = 5
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);
    // bit = 4
    f_mul(t[0], t[0], t[0]);
    // bit = 3
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);
    // bit = 2
    f_mul(t[0], t[0], t[0]);
    // bit = 1
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);
    // bit = 0
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);

    f_copy(t[0], a);  
#endif
}

void f_leg(const f_elm_t a, unsigned char *b)
{
#if (NBITS_FIELD == 128)
    unsigned int i;
    f_elm_t t0;

    f_copy(Mont_one, t0);

    /* (p - 1)/2 = 0b   0100000000000000\
                        0000000000000000\
                        0000000000000000\
                        0000000000000000\
                        0000000000000000\
                        0000000000000000\
                        0000000000000000\
                        0000000000001110\
    */

    // Compute a^((p-1)/2)

    // bit = 127 (=0)

    // bit = 126
    f_mul(t0, a, t0);

    // bits 125 down to 4
    for (i = 125; i > 3; i--)
    {
        f_mul(t0, t0, t0);
    }

    // bit = 3
    f_mul(t0, t0, t0);
    f_mul(t0, a, t0);

    // bit = 2
    f_mul(t0, t0, t0);
    f_mul(t0, a, t0);

    // bit = 1
    f_mul(t0, t0, t0);
    f_mul(t0, a, t0);

    // bit = 0
    f_mul(t0, t0, t0);

    *b = ((*(unsigned char *)t0) & 0x08) >> 3;
#endif

#if (NBITS_FIELD == 192)
    f_elm_t t[5];
    unsigned int i, j;

    f_copy(a, t[0]);
    f_copy(a, t[1]);

    /* (p - 1)/2 = 0b   0 1111111 11111111
                        11111111 11111111
                        11111111 11111111
                        11111111 11111111
                        11111111 11111111
                        11111111 11111111
                        11111111 11111111
                        11111111 11111111
                        11111111 11111111
                        11111111 11111111
                        11111111 11111111
                        11111111 1 0001001\
    */   

    // First 128 bits = 2^7 bits
    for(j = 0; j < 7; j++){
        for (i = 0; i < (1 << j); i++)
            f_mul(t[0], t[0], t[0]);
        f_mul(t[0], t[1], t[0]);
        if(j == 2) f_copy(t[0], t[2]);  // t[2] = a^(2^16-1) = a^0b 11111111
        if(j == 3) f_copy(t[0], t[3]);  // t[3] = a^(2^16-1) = a^0b 11111111 11111111
        if(j == 4) f_copy(t[0], t[4]);  // t[4] = a^(2^16-1) = a^0b 11111111 11111111 11111111 11111111
        f_copy(t[0], t[1]);             // = a^(2^(2^(j+1)) - 1)
    }
    
    /* t[3] = a ^ 0b 11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
    */

    // Next 32 bits
    for (i = 0; i < 32; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[4], t[0]);

    // Next 16 bits
    for (i = 0; i < 16; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[3], t[0]);

    // Next 8 bits
    for (i = 0; i < 8; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[2], t[0]);


    // bit = 6
    f_mul(t[0], t[0], t[0]);
    // bit = 5
    f_mul(t[0], t[0], t[0]);
    // bit = 4
    f_mul(t[0], t[0], t[0]);
    // bit = 3
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);
    // bit = 2
    f_mul(t[0], t[0], t[0]);
    // bit = 1
    f_mul(t[0], t[0], t[0]);
    // bit = 0
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);

    *b = ((*(unsigned char *)(t[0])) & 0x02) >> 1;
#endif

#if (NBITS_FIELD == 256)
     f_elm_t t[6];
    unsigned int i, j;

    f_copy(a, t[0]);
    f_copy(a, t[1]);

    /* (p - 1)/2 = 0b   00 111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11111111\
                        11111111 11 110110\
    */
    // Compute a^(p-2)
    // bit 255 = 0

    // First 128 bits = 2^7 bits
    for(j = 0; j < 7; j++){
        for (i = 0; i < (1 << j); i++)
            f_mul(t[0], t[0], t[0]);
        f_mul(t[0], t[1], t[0]);
        if(j == 2) f_copy(t[0], t[2]);  // t[2] = a^(2^8  - 1) = a^0b 11111111
        if(j == 3) f_copy(t[0], t[3]);  // t[3] = a^(2^16 - 1) = a^0b 11111111 11111111
        if(j == 4) f_copy(t[0], t[4]);  // t[4] = a^(2^32 - 1) = a^0b 11111111 11111111 11111111 11111111
        if(j == 5) f_copy(t[0], t[5]);  // t[5] = a^(2^64 - 1) = a^0b 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
        f_copy(t[0], t[1]);             // = a^(2^(2^(j+1)) - 1)
    }
    
    /* t[3] = a ^ 0b 11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
                     11111111 11111111
    */

    // Next 64 bits
    for (i = 0; i < 64; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[5], t[0]);

    // Next 32 bits
    for (i = 0; i < 32; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[4], t[0]);

    // Next 16 bits
    for (i = 0; i < 16; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[3], t[0]);

    // Next 8 bits
    for (i = 0; i < 8; i++)
        f_mul(t[0], t[0], t[0]);
    f_mul(t[0], t[2], t[0]);

    // bit = 5
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);
    // bit = 4
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);
    // bit = 3
    f_mul(t[0], t[0], t[0]);
    // bit = 2
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);
    // bit = 1
    f_mul(t[0], t[0], t[0]);
    f_mul(t[0], a, t[0]);
    // bit = 0
    f_mul(t[0], t[0], t[0]);

    *b = ((*(unsigned char *)t[0]) & 0x01);
#endif
}
