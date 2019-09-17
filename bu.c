#include <string.h> // for memset, etc.
#include <stdio.h>  // for printf
#include <string.h>

#include "bu.h"

// NOTE: In this code "word" always refers to a uint32_t

// Copy dest = src
void bu_cpy(bigunsigned *dest, bigunsigned *src) {
  uint16_t cnt = src->used;
  dest->used = cnt;
  dest->base = 0;

  // reset upper 0s in dest
  memset(dest->digit, 0, sizeof(uint32_t)*BU_DIGITS-cnt);

  uint8_t i_dest = dest->used; // TODO: This is wrong. Fix it.
  uint8_t i_src = src->used;

  while (cnt-- > 0) {
    dest->digit[--i_dest] = src->digit[src->base+(--i_src)];
  }
}

// Set to 0
void bu_clear(bigunsigned *a_ptr) {
  memset(a_ptr->digit, 0, sizeof(uint32_t)*BU_DIGITS);
  a_ptr->used = 0;
  a_ptr->base = 0;
}

//a = b<<cnt
void bu_shl(bigunsigned* a_ptr, bigunsigned* b_ptr, uint16_t cnt) {
  if(cnt == 0){
    return;
  }
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  uint32_t mask = 0xffffffff << (BU_BITS_PER_DIGIT - bits);
  if (bits == 0){
    mask = 0; // This was because I was getting a real weird bug where if bits was 0 then mask would not change
  }

  // You implement. Avoid memory copying as much as possible
  
  //First I am gonna check for garbage in A that I don't want (I have to use int else it will run forever)
  a_ptr->base = b_ptr->base;
  a_ptr->used = b_ptr->used;
  for (uint16_t i = b_ptr->used; i < BU_DIGITS; i += 1){
    a_ptr->digit[(uint8_t)(a_ptr->base+i)] = 0;
  }

  //I am doing a for loop because it will take care of both moving the base for the words and handle possible overflow
  for (uint16_t i = 0; i < wrds; i += 1){
    a_ptr->digit[a_ptr->base - 1] = 0; //Sets the most siginificant digit to 0
  }
  a_ptr->base = a_ptr->base - wrds;

  a_ptr->used = BU_DIGITS <= (b_ptr->used + wrds) 
                ? BU_DIGITS : (b_ptr->used + wrds); //Makes sure that used is bound by digits available

  //Now for the within digit shifts
  uint32_t carry = 0;                          //The bits that need to be carried (I have it separated for my own readability)
  uint8_t astart = a_ptr->base + a_ptr->used ;                                     //Where the loop starts for a
  uint8_t offset = b_ptr->used + wrds <= BU_DIGITS ?                              //Offset is the difference in written on used digits
                   b_ptr->used : b_ptr->used - (b_ptr->used + wrds) % BU_DIGITS;  //between b and a, it will give us which digit we start
  uint8_t bstart = b_ptr->base + offset;                                          //The shift loop on

  for (uint8_t i = 0; i <= offset; i += 1) {
    carry = (b_ptr->digit[bstart - i - 1] & mask) >> (BU_BITS_PER_DIGIT - bits);
    a_ptr->digit[astart - i] = (b_ptr->digit[bstart - i] << bits) | carry;
  }

  //Last check to see if during the shift some bits went into a new digit:
  if (a_ptr->digit[(uint8_t)(a_ptr->used + a_ptr->base)] != 0 && a_ptr->used < BU_DIGITS){
    a_ptr->used += 1;
  }
  
}

// Shift in place a bigunsigned by cnt bits to the left
// Example: beef shifted by 4 results in beef0
void bu_shl_ip(bigunsigned* a_ptr, uint16_t cnt) {
  bu_shl(a_ptr,a_ptr,cnt);
}

//a = (b>>cnt)
void bu_shr(bigunsigned* a_ptr,  bigunsigned* b_ptr, uint16_t cnt){
  if(cnt == 0){
    return;
  }
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
  for (uint8_t i = 0; i < wrds; i += 1) {
    a_ptr->digit[a_ptr->base] = 0;
    a_ptr->base = a_ptr->base + 1;
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

//a += b
//This is probably not as fast as the regular add but the code is nicer
//Should not be used if the result > 8k Bits
void bu_add_ip(bigunsigned *a_ptr, bigunsigned *b_ptr){
  uint64_t carry = 0;
  uint16_t bound = a_ptr->used >= b_ptr->used ? a_ptr->used : b_ptr->used; 

  for (uint16_t i = 0; i < bound; i+=1){
    carry = (uint64_t)(b_ptr->digit[(uint8_t)(i+b_ptr->base)]) + (uint64_t)(a_ptr->digit[(uint8_t)(i+a_ptr->base)]) + (carry >> 32);
    a_ptr->digit[(uint8_t)(a_ptr->base+i)] = (uint32_t)carry;
  }
  a_ptr->used = bound;

  if (a_ptr->digit[(uint8_t)(a_ptr->used+a_ptr->base)] != 0 && a_ptr->used < BU_DIGITS){
    a_ptr->used += 1;
  }

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

//a = b*d
//This should not be used if the result would be greater that 8k bits or it will give a weird answer
void bu_mul_digit(bigunsigned *a_ptr, bigunsigned *b_ptr, uint32_t d){
  bigunsigned carry; //carry will make sure I only have to add on the last step
  bu_clear(&carry);  //just to make sure there isn't any junk in this variable
  uint64_t mult = 0; //This is a temporary variable that holds the digit multiplication

  for(uint16_t i = b_ptr->used; i < BU_DIGITS; i+=1){
    a_ptr->digit[(uint8_t)(a_ptr->base+i)] = 0; //To clear the memory that shouldn't be non-zero out without clearing everything
  } 

  for (uint16_t i = 0; i < b_ptr->used; i += 1){
    mult = (uint64_t)(b_ptr->digit[(uint8_t)(b_ptr->base+i)])*(uint64_t)(d);  //calculate the full multiplication of the two digits
    a_ptr->digit[(uint8_t)(a_ptr->base+i)] = (uint32_t)mult;                  //put the low digit in the array in the spot it needs to be
    carry.digit[(uint8_t)(i+1)] = (uint32_t)(mult >> 32);                     //Hold the larger digit for later
  }

  a_ptr->used = b_ptr->used;
  carry.used = a_ptr->used + 1;


  while (a_ptr->digit[(uint8_t)(a_ptr->used+a_ptr->base)] != 0 && a_ptr->used < BU_DIGITS){
    a_ptr->used += 1;
  }

  bu_add_ip(a_ptr,&carry); //finally add the carry to the non-carried data

  if (a_ptr->digit[(uint8_t)(a_ptr->used+a_ptr->base-1)] == 0 && a_ptr->used > 0){
    a_ptr->used -= 1; //This fixed a weird bug (All bugs are weird)
  }
  
}

// a *= d
void bu_mul_digit_ip(bigunsigned *a_ptr, uint32_t d){
  bu_mul_digit(a_ptr,a_ptr,d);
}

// a = b*c
//There is unstable behavior for results greater than 8k bits
void bu_mul(bigunsigned *a_ptr, bigunsigned *b_ptr, bigunsigned *c_ptr){  
  bigunsigned running; //Temporary running total
  bu_clear(&running);
  bigunsigned carry; //Temporary for one line multiplication
  bu_clear(&carry);
  uint32_t sh = 0;
  
  for(uint16_t i = 0; i < b_ptr->used; i+=1){
    bu_mul_digit(&carry,c_ptr,b_ptr->digit[(uint8_t)(b_ptr->base+i)]);  //Calculate the multiplication of a digit to the other number
    sh = ((uint32_t)i) << 5;
    bu_shl_ip(&carry,sh); //Shift the thing to the right place
    printf("\n Digit num: %x \n", i);
    printf("\n Digit val: %x \n", b_ptr->digit[(uint8_t)(b_ptr->base+i)]);
    bu_dbg_printf(&carry);
    bu_add_ip(&running,&carry); //Add this line to the running total
  }

  bu_cpy(a_ptr,&running); //copy the running total into the ptr we want it in
}

// a *= b
void bu_mul_ip(bigunsigned *a_ptr, bigunsigned *b_ptr){
  bu_mul(a_ptr,a_ptr,b_ptr);
}

// a = b^2
void bu_sqr(bigunsigned *a_ptr, bigunsigned *b_ptr){
  bu_mul(a_ptr,b_ptr,b_ptr);
}

// a *= a
void bu_sqr_ip(bigunsigned *a_ptr) {
  bu_mul_ip(a_ptr,a_ptr);
}

// Read from a string of hex digits
//
// TODO: This is wrong. See the test main.c
//       Modify to resolve 'endian' conflict.
//       Also modify to permit strings to include whitespace
//        that will be ignored. For example, "DEAD BEEF" should
//        be legal input resulting in the value 0xDEADBEEF.

void bu_readhex(bigunsigned *a_ptr, char *s) {
  bu_clear(a_ptr);

  //Reverse method:
  char *s_ptr = s;
  int count = strlen(s_ptr)-1;
  unsigned pos = 0;
  
  while (count >= 0 && pos < BU_MAX_HEX) {
    if (!isspace(s[count])) {
      a_ptr->digit[pos>>3] |= (((uint32_t)hex2bin(s[count])) << ((pos & 0x7)<<2));
      pos++;
    }
    count -= 1;
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
    printf("%8x ", a_ptr->digit[(uint8_t)(a_ptr->base+i)]);
  printf("Length: %x\n", bu_len(a_ptr));
}
