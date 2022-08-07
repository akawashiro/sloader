#include <assert.h>
#include <stdint.h>

#include "/home/akira/sloader/raw_write.h"

extern __thread uint32_t i;
extern uint32_t j;

int main() {
  RAW_PRINT_STR("__thread uint32_t i=");
  RAW_PRINT_HEX(i);
  RAW_PRINT_STR("\n&i=");
  RAW_PRINT_HEX(&i);

  RAW_PRINT_STR("\nShould be 0xaabbccdd: uint32_t j=");
  RAW_PRINT_HEX(j);
  RAW_PRINT_STR("\n&j=");
  RAW_PRINT_HEX(&j);
  return 0;
}
