#include "raw_write.h"

unsigned long hoge_var = 0xdeadbeef;

// Ooops!
// We must define buf here as a global variable not as a local variable in
// print_hoge_var. Otherwise, the shared object depends on libc.so. I don't know
// the reason.
char buf[9];

int hoge(int a, int b) {
    RAW_PRINT_STR("Hello World!\n");
    return a + b;
}

void print_hoge_var() {
    // Simple itoa
    for (int i = 0; i < 8; i++) {
        char c = (hoge_var >> (4 * (7 - i))) & (0xf);
        c += (c < 10) ? '0' : 'a' - 10;
        buf[i] = c;
    }
    buf[8] = 0;

    RAW_PRINT_STR("hoge_var = ");
    RAW_PRINT_STR(buf);
    RAW_PRINT_STR("\n");
}
