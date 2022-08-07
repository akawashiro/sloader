#include "raw_write.h"

// Ooops!
// We must define buf here as a global variable not as a local variable in
// print_hoge_var. Otherwise, the shared object depends on libc.so. I don't know
// the reason.
char buf[9];

struct Hoge {
    int idx;
    char buf[16];
    void* p;
};

static struct Hoge hoge;

void print_hex(unsigned long var) {
    // Simple itoa
    for (int i = 0; i < 8; i++) {
        char c = (var >> (4 * (7 - i))) & (0xf);
        c += (c < 10) ? '0' : 'a' - 10;
        buf[i] = c;
    }
    buf[8] = 0;

    RAW_PRINT_STR(buf);
    RAW_PRINT_STR("\n");
}

void print_hoge_var() {
    hoge.idx = 0xdeadbeef;
    RAW_PRINT_STR("hoge.idx=0x");
    print_hex(hoge.idx);
    RAW_PRINT_STR("\n");
}
