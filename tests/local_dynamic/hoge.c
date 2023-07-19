#include <stdint.h>
#include "raw_write.h"

static __thread uint32_t i0 = 0xdeadbeef;
static __thread uint32_t i1 = 0xeeeeeeee;

void f() {
    RAW_PRINT_STR("__thread uint32_t i0=");
    RAW_PRINT_HEX(i0);
    RAW_PRINT_STR("\n&i0=");
    RAW_PRINT_HEX(&i0);
    RAW_PRINT_STR("\n");

    RAW_PRINT_STR("__thread uint32_t i1=");
    RAW_PRINT_HEX(i1);
    RAW_PRINT_STR("\n&i1=");
    RAW_PRINT_HEX(&i1);
    RAW_PRINT_STR("\n");
}
