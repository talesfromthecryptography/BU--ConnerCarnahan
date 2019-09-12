#include <stdio.h>
#include <string.h>

#include "bu.h"

int main() {
  bigunsigned a,b,c;
  char s[BU_MAX_HEX+1];

  bu_readhex(&a,"CAB51AFF BEEF");
  bu_readhex(&b,"111111111111");

  bu_add(&c, &a, &b);
 
  bu_dbg_printf(&c);
  
  
  //For testing shifts
  
  printf("\n \n Shift a right by 16\n");
  bu_shr_ip(&a,16);
  bu_dbg_printf(&a);

  bu_readhex(&a,"CAB51AFF BEEF");
  printf("\n \n Shift a right by 36\n");
  bu_shr_ip(&a,36);
  bu_dbg_printf(&a);

  //bu_shl(&b,&c,32);
  //bu_dbg_printf(&b);

  bu_readhex(&a,"CAB51AFF BEEF");
  printf("\n \n Shift a left by 16\n");
  bu_shl_ip(&a,16);
  bu_dbg_printf(&a);

  bu_readhex(&a,"CAB51AFF BEEF");
  printf("\n \n Shift a left by 32\n");
  bu_shl_ip(&a,32);
  bu_dbg_printf(&a);
  
  bu_readhex(&a,"CAB51AFF BEEF");
  printf("\n \n Shift a left by 56\n");
  bu_shl_ip(&a,56);
  bu_dbg_printf(&a);

  bu_readhex(&a,"CAB51AFF BEEF");
  printf("\n \n Shift a left by 64\n");
  bu_shl_ip(&a,64);
  bu_dbg_printf(&a); 


  return 0;
}