#ifndef test43_Class_Defined__
#define test43_Class_Defined__

#include <stdint.h>
#include "test01.h"

class test43 {
public:
  static const int cols = 40;
  static const int rows = 12;
  static const int screensize = 480;
  static const int lastrow = 440;
  test01	a;
  int32_t	getx(void);
private:
};

#endif
