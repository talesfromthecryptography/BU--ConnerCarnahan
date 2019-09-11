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


void bu_shl(bigunsigned* a_ptr, bigunsigned* b_ptr, uint16_t cnt) {
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  uint32_t mask = 0xffffffff << (BU_BITS_PER_DIGIT - bits);

  // You implement. Avoid memory copying as much as possible

  //First I am gonna check for garbage in A that I don't want
  a_ptr->base = b_ptr->base;
  for (int i = b_ptr->used; i <= BU_DIGITS; i+= 1){
    a_ptr->digit[(a_ptr->base+i)%BU_DIGITS] = 0;
  }

  //This is for having big overflows
  for (int i = BU_DIGITS; i < b_ptr->used + wrds; i += 1){
    
  }

  //Now for the within digit shifts
  uint32_t carry = 0;
  uint8_t index = 0;
  for (int i = 0; i <= b_ptr->used; i+=1){
    index = (b_ptr->base + b_ptr->used - i) % BU_DIGITS;
    
  }

}

// Shift in place a bigunsigned by cnt bits to the left
// Example: beef shifted by 4 results in beef0
void bu_shl_ip(bigunsigned* a_ptr, uint16_t cnt) {
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  uint32_t mask = 0xffffffff << bits;

  // You implement. Avoid memory copying as much as possible.
  uint32_t carryOff = 0;
  uint32_t carryOn = 0; 

}

//a = (b>>cnt)
void bu_shr(bigunsigned* a_ptr,  bigunsigned* b_ptr, uint16_t cnt){
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  uint32_t mask = 0xffffffff >> (BU_BITS_PER_DIGIT - bits);

  //First to check if we are larger than the total amount of used for b so that we can just clear the thing instead
  if (wrds >= b_ptr->used) {
    bu_clear(a_ptr);
    return;
  }
  
  //Then I need to make sure a_ptr doesn't have random garbage in it:
  for (int i = b_ptr->used; i < BU_DIGITS; i+= 1) {
    a_ptr->digit[(b_ptr->base+i)%BU_DIGITS] = 0;
  }

  //Then to take care of the shifts by digits ( I do all this on A so that I can also just pass b as a and we will get 
  //the shift in place and so that if A had stuff in it before it will be correct and then okay cool)
  a_ptr->base = b_ptr->base;
  for (int i = 0; i < wrds; i += 1) {
    a_ptr->digit[a_ptr->base] = 0;
    a_ptr->base = (a_ptr->base + 1) % BU_DIGITS;
    a_ptr->used -= 1;
  }

  //Now for the shifts within the digits
  uint32_t carry = 0;
  uint8_t index = 0;
  for (int i = 0; i < a_ptr->used; i += 1) {
    index = (a_ptr->base+i)%BU_DIGITS;
    carry = ((mask & b_ptr->digit[(index+1)%BU_DIGITS]) << (BU_BITS_PER_DIGIT-bits));
    a_ptr->digit[index] = (b_ptr->digit[index] >> bits) | carry;
  }

  //last I am gonna have it check to see if a bit was shifted over in the last step (like if it was over a boundary)
  if (a_ptr->used > 0) {
    if (a_ptr->digit[(a_ptr->used+a_ptr->base-1)%BU_DIGITS] == 0){
      a_ptr->used -= 1;
    }
  }


}

//  a >>= cnt
// Shift in place a big unsigned by cnt bits to the left
// Example: beef shifted by 4 results in beef0
void bu_shr_ip(bigunsigned* a_ptr, uint16_t cnt){
  bu_shr(a_ptr, a_ptr, cnt);
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

  char *s_ptr = s;
  unsigned pos = 0;

  while (*s_ptr && pos < BU_MAX_HEX) {
    if (!isspace(*s_ptr)) {
      a_ptr->digit[pos>>3] |= (((uint32_t)hex2bin(*s_ptr)) << ((pos & 0x7)<<2));
	    //a_ptr->digit[0] |= (uint32_t)hex2bin(*s_ptr);
	    //bu_shl_ip(a_ptr, 4);
      pos++;
    }
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
