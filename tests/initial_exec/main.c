#include <assert.h>
#include <stdint.h>

#include "/home/akira/sloader/raw_write.h"

extern __thread uint32_t i0;
extern __thread uint32_t i1;
extern __thread uint32_t i2;
extern __thread uint32_t i3;
extern uint32_t j;

void set_i0() { i0 = 0xaabbccdd; }

int main() {
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

    RAW_PRINT_STR("__thread uint32_t i2=");
    RAW_PRINT_HEX(i2);
    RAW_PRINT_STR("\n&i2=");
    RAW_PRINT_HEX(&i2);
    RAW_PRINT_STR("\n");

    RAW_PRINT_STR("__thread uint32_t i3=");
    RAW_PRINT_HEX(i3);
    RAW_PRINT_STR("\n&i3=");
    RAW_PRINT_HEX(&i3);
    RAW_PRINT_STR("\n");

    RAW_BREAK();
    i0 = 0xeeeeeeee;
    i1 = 0xffffffff;

    RAW_PRINT_STR("\nShould be 0xaabbccdd: uint32_t j=");
    RAW_PRINT_HEX(j);
    RAW_PRINT_STR("\n&j=");
    RAW_PRINT_HEX(&j);
    RAW_PRINT_STR("\n");
    return 0;
}
