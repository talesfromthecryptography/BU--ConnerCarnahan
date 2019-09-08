#include <string.h> // for memset, etc.
#include <stdio.h>  // for printf

#include "bu.h"

// NOTE: In this code "word" always refers to a uint32_t

// Copy dest = src
void bu_cpy(bigunsigned *dest, bigunsigned *src) {
  uint16_t cnt = src->used;
  dest->used = cnt;
  dest->base = 0;

  // reset upper 0s in dest
  memset(dest->digit, 0, sizeof(uint32_t)*BU_DIGITS-cnt);

  uint8_t i_dest = 0; // TODO: This is wrong. Fix it.
  uint8_t i_src = src->base;

  while (cnt-- > 0) {
    dest->digit[i_dest--] = src->digit[i_src--];
  }
}

// Set to 0
void bu_clear(bigunsigned *a_ptr) {
  memset(a_ptr->digit, 0, sizeof(uint32_t)*BU_DIGITS);
  a_ptr->used = 0;
  a_ptr->base = 0;
}

// Shift in place a bigunsigned by cnt bits to the left
// Example: beef shifted by 4 results in beef0
void bu_shl_ip(bigunsigned* a_ptr, uint16_t cnt) {
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  uint32_t mask = 0xffffffff << bits;

  // You implement. Avoid memory copying as much as possible.

  //This loops from largest to smallest copying over the bits that need to shift
  //I ensure that the modulus remains positive (else C gets weird with negative modulus) by adding BU_DIGITS at the front
  for (int i = 0; i < used; i += 1) {

	  //First I grab the bits that need to move from one digit to the next and then or them to the larger digit
	  a_ptr->digit[(BU_DIGITS + used - i + 1 + a_ptr->base) % BU_DIGITS] |= 
		  ((mask & a_ptr->digit[(BU_DIGITS + used - i + a_ptr->base) % BU_DIGITS]) >> (BU_BITS_PER_DIGIT - bits));

	  //Then I shift the bits in the digit
	  a_ptr->digit[(BU_DIGITS + used - i + a_ptr->base) % BU_DIGITS] <<= bits;

  }

  //Then I shift the base back the number of wrds I want and update the used number
  a_ptr->base = (BU_DIGITS + a_ptr->base-wrds)% BU_DIGITS;
  a_ptr->used += wrds;

  //Then I make sure that used is updated
  if (a_ptr->digit[(BU_DIGITS +base + used + 1) % BU_DIGITS] != 0) {
	  used += 1;
  }

  //Now I have to make a decision about overflow, I am opting to have it set so that it loops back to 0, cause that makes the most sense to me
  uint8_t loopnum = a_ptr->used <= BU_DIGITS-1 ? a_ptr->used : BU_DIGITS-1;
  uint8_t setbacknum = 0;

  for (int i = 0; i <= loopnum; i += 1) {
	  if (a_ptr->digit[(BU_DIGITS + a_ptr->used + a_ptr->base - i) % BU_DIGITS] == 0) {
		  setbacknum += 1;
	  }
	  else {
		  break;
	  }
  }
  a_ptr->used -= setbacknum;
}

// Produce a = b + c
void bu_add(bigunsigned *a_ptr, bigunsigned *b_ptr, bigunsigned *c_ptr) {
  uint8_t carry = 0;
  uint64_t nxt;
  uint16_t cnt = 0;
  uint16_t min_used = b_ptr->used <= c_ptr->used 
                      ? b_ptr->used : c_ptr->used;
  uint8_t  b_dig = b_ptr->base;
  uint8_t  c_dig = c_ptr->base;
  uint8_t  a_dig = 0;

  while (cnt < min_used) {
    nxt = ((uint64_t)b_ptr->digit[b_dig++]) 
          + (uint64_t)(c_ptr->digit[c_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < b_ptr->used && carry) {
    nxt = ((uint64_t)b_ptr->digit[b_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < b_ptr->used) {
    a_ptr->digit[a_dig++] = b_ptr->digit[b_dig++];
    cnt++;
  }  

  while (cnt < c_ptr->used && carry) {
    nxt = ((uint64_t)c_ptr->digit[c_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < c_ptr->used) {
    a_ptr->digit[a_dig++] = c_ptr->digit[c_dig++];
    cnt++;
  }

  while (cnt < BU_DIGITS && carry) {
    a_ptr->digit[a_dig++] = 1;
    carry = 0;
    cnt++;
  }

  a_ptr->base = 0;
  a_ptr->used = cnt;
}

// return the length in bits (should always be less or equal to 32*a->used)
uint16_t bu_len(bigunsigned *a_ptr) {
  uint16_t res = a_ptr->used<<5;
  uint32_t bit_mask = 0x80000000;
  uint32_t last_wrd = a_ptr->digit[a_ptr->base+a_ptr->used-1];

  while (bit_mask && !(last_wrd&bit_mask)) {
    bit_mask >>= 1;
    res--;
  }
  return res;
}

// Read from a string of hex digits
//
// TODO: This is wrong. See the test main.c
//       Modify to resolve 'endian' conflict.
//       Also modify to permit strings to include whitespace
//        that will be ignored. For example, "DEAD BEEF" should
//        be legal input resulting in the value 0xDEADBEEF.

void bu_readhex(bigunsigned * a_ptr, char *s) {
  bu_clear(a_ptr);

  //To clear whitespace
  do {
	  while (isspace(*s) {
		  s++;
	  }
  } while (*s++)

  unsigned pos = 0;
  char *s_ptr = s;
  unsigned length = strlen(s)>>1;

  while (*s_ptr && pos < BU_MAX_HEX) {
    a_ptr->digit[length - pos>>3] |= (((uint32_t)hex2bin(*s_ptr)) << ((pos & 0x7)<<2));
    pos++;
    s_ptr++;
  }
  a_ptr->used = (pos>>3) + ((pos&0x7)!=0);
}

// 
void bu_dbg_printf(bigunsigned *a_ptr) {
  printf("Used %x\n", a_ptr->used);
  printf("Base %x\n", a_ptr->base);
  uint16_t i = a_ptr->used;
  printf("Digits: ");
  while (i-- > 0)
    printf("%8x ", a_ptr->digit[a_ptr->base+i]);
  printf("Length: %x\n", bu_len(a_ptr));
}
