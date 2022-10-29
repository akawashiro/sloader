#include "raw_write.h"
#include <stdint.h>

static __thread uint32_t i0 = 0xdeadbeef;

void f(){
    RAW_PRINT_STR("__thread uint32_t i0=");
    RAW_PRINT_HEX(i0);
    RAW_PRINT_STR("\n&i0=");
    RAW_PRINT_HEX(&i0);
    RAW_PRINT_STR("\n");
}
