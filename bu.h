#ifndef _BU_H
#define _BU_H

//#include <stdio.h>
//#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define BU_DIGITS 256
#define BU_MAX_HEX (BU_DIGITS<<3)
#define BU_BITS (BU_DIGITS<<5)
#define BU_BITS_PER_DIGIT 32

typedef struct {
  uint32_t digit[BU_DIGITS];
  uint16_t used;  // number of non-leading zero digits
  uint8_t base;  // start from this index
} bigunsigned;

// Copy dest = src
void bu_cpy(bigunsigned *dest, bigunsigned *src);

// Set to 0
void bu_clear(bigunsigned *a_ptr);

//  a = b<<cnt
void bu_shl(bigunsigned* a_ptr, bigunsigned* b_ptr, uint16_t cnt);
//  a <<= cnt
// Shift in place a big unsigned by cnt bits to the left
// Example: beef shifted by 4 results in beef0
void bu_shl_ip(bigunsigned* a_ptr, uint16_t cnt);

//  a = b>>cnt
void bu_shr(bigunsigned* a_ptr,  bigunsigned* b_ptr, uint16_t cnt);
//  a >>= cnt
// Shift in place a big unsigned by cnt bits to the left
// Example: beef shifted by 4 results in beef0
void bu_shr_ip(bigunsigned* a_ptr, uint16_t cnt);

//  a = b + c
void bu_add(bigunsigned *a_ptr, bigunsigned *b_ptr, bigunsigned *c_ptr);
//  a += b
void bu_add_ip(bigunsigned *a_ptr, bigunsigned *b_ptr);
// return the length in bits (should always be less or equal to 32*a->used)


// a = b*d
void bu_mul_digit(bigunsigned *a_ptr, bigunsigned *b_ptr, uint32_t d);
// a *= d
void bu_mul_digit_ip(bigunsigned *a_ptr, uint32_t d);

// a = b*c
void bu_mul(bigunsigned *a_ptr, bigunsigned *b_ptr, bigunsigned *c_ptr);
// a *= b
void bu_mul_ip(bigunsigned *a_ptr, bigunsigned *b_ptr);

// a = b^2
void bu_sqr(bigunsigned *a_ptr, bigunsigned *b_ptr);
// a *= a
void bu_sqr_ip(bigunsigned *a_ptr);

// Count of the bits, excluding leading 0s
uint16_t bu_len(bigunsigned *a_ptr);

// Convert single ascii character hex digit to numerical value
// Assumes given a true hex digit, can be lower or upper case
// Does not check for correct input


// Convert single ascii character hex digit to numerical value
// Assumes given a true hex digit, can be lower or upper case
// Does not check for correct input
static inline uint8_t hex2bin(char c) {
  if ('0' <= c && c <= '9')
    return c - '0';
  else
    return tolower(c)-'a' + 0xa;
}

// Read from a string of hex digits
//
// TODO: This is wrong. See the test main.c
//       Modify to resolve 'endian' conflict.
//       Also modify to permit strings to include whitespace
//        that will be ignored. For example, "DEAD BEEF" should
//        be legal input resulting in te value 0xDEADBEEF.

void bu_readhex(bigunsigned * a_ptr, char *s);
// 
void bu_dbg_printf(bigunsigned *a_ptr);

#endif
