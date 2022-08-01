#include <assert.h>
#include <stdint.h>

#include "/home/akira/sloader/raw_write.h"

__thread uint32_t i0 = 0xdeadbeef;
__thread uint32_t i1 = 0xabcdabcd;
__thread uint32_t i2 = 0x01234567;
__thread uint32_t i3 = 0x90abcdef;
__thread uint32_t i4;
__thread uint32_t i5;
// uint32_t j = 0xaabbccdd;

void set() { i0 = 0xabcdabcd; }

int main() {
    // RAW_BREAK();
    // i0 = 0xdeadbeef;

    RAW_PRINT_STR("__thread uint32_t i0=");
    RAW_PRINT_HEX(i0);
    RAW_PRINT_STR("\n");
    RAW_PRINT_STR("__thread uint32_t i1=");
    RAW_PRINT_HEX(i1);
    RAW_PRINT_STR("\n");
    RAW_PRINT_STR("__thread uint32_t i2=");
    RAW_PRINT_HEX(i2);
    RAW_PRINT_STR("\n");
    RAW_PRINT_STR("__thread uint32_t i3=");
    RAW_PRINT_HEX(i3);
    RAW_PRINT_STR("\n");
    RAW_PRINT_STR("__thread uint32_t i4=");
    RAW_PRINT_HEX(i4);
    RAW_PRINT_STR("\n");
    RAW_PRINT_STR("__thread uint32_t i5=");
    RAW_PRINT_HEX(i5);
    RAW_PRINT_STR("\n");

    RAW_PRINT_STR("&i0=0x");
    RAW_PRINT_HEX(&i0);
    RAW_BREAK();
    RAW_PRINT_STR("\n&i1=");
    RAW_PRINT_HEX(&i1);
    RAW_PRINT_STR("\n&i2=");
    RAW_PRINT_HEX(&i2);
    RAW_PRINT_STR("\n&i3=");
    RAW_PRINT_HEX(&i3);
    RAW_PRINT_STR("\n&i4=");
    RAW_PRINT_HEX(&i4);
    RAW_PRINT_STR("\n&i5=");
    RAW_PRINT_HEX(&i5);

    // RAW_PRINT_STR("\nShould be 0xaabbccdd: uint32_t j=");
    // RAW_PRINT_HEX(j);
    // RAW_PRINT_STR("\n&j=");
    // RAW_PRINT_HEX(&j);
    return 0;
}
