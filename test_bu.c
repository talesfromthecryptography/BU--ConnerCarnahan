#include <stdio.h>
#include <string.h>

#include "bu.h"

int main() {
  bigunsigned a,b,c,d;

  bu_readhex(&a,"BEEFBEEFBEEFBEEFBEEF");
  bu_readhex(&b,"10000000000000000001");
  bu_readhex(&d,"1");

  bu_add(&c, &a, &b);
 
  bu_dbg_printf(&b);
  //bu_cpy(&d,&a);
  bu_mul(&d,&a,&b);
  //bu_mul_digit(&d,&a,b.digit[0]);
  bu_dbg_printf(&d);

  return 0;
}