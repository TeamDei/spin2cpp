#include <propeller.h>
#include "test50.h"

int32_t test50::myinit(int32_t a, int32_t b, int32_t c)
{
  int32_t result = 0;
  int32_t _parm__0000[] = { a, b, c };
  memmove(&gb, &_parm__0000[1], 4*(2));
  return result;
}

