#include <assert.h>
#include <stdint.h>

#include "/home/akira/sloader/raw_write.h"

__thread uint32_t i = 0xdeadbeef;
uint32_t j = 0xaabbccdd;

int main() {
    RAW_PRINT_STR("Should be 0xdeadbeef: __thread uint32_t i=");
    RAW_PRINT_HEX(i);
    RAW_PRINT_STR("\n&i=");
    RAW_PRINT_HEX(&i);

    RAW_PRINT_STR("\nShould be 0xaabbccdd: uint32_t j=");
    RAW_PRINT_HEX(j);
    RAW_PRINT_STR("\n&j=");
    RAW_PRINT_HEX(&j);
    return 0;
}
