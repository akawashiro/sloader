#include "raw_write.h"

void hoge(int a, int b);

void fuga() {
  RAW_PRINT_STR("Hello World! from fuga\n");
  hoge(40, 2);
  return;
}
