#include <propeller.h>
#include "btest003.h"

void btest003::program(void)
{
  ser.start(31, 30, 0, 115200);
  for(i = 1; ((uint32_t)(i)) < 11; i++) {
    ser.dec(i);
    ser.tx(13);
    ser.tx(10);
  }
  waitcnt(0);
}

