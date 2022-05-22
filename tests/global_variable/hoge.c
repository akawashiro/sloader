#include "raw_write.h"

int hoge_var = 0;

// Ooops!
// We must define buf here as a global variable not as a local variable in
// print_hoge_var. Otherwise, the shared object depends on libc.so. I don't know
// the reason.
char buf[4];

int hoge(int a, int b) {
    RAW_PRINT_STR("Hello World!\n");
    return a + b;
}

void print_hoge_var() {
    // Simple itoa
    buf[0] = (hoge_var % 1000) / 100 + '0';
    buf[1] = (hoge_var % 100) / 10 + '0';
    buf[2] = hoge_var % 10 + '0';
    buf[3] = 0;

    RAW_PRINT_STR("hoge_var = ");
    RAW_PRINT_STR(buf);
    RAW_PRINT_STR("\n");
}
