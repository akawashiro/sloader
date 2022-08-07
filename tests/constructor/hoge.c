#include "raw_write.h"

void before_main() __attribute__((constructor));
void before_main() { RAW_PRINT_STR("====== Before main ======\n"); }

unsigned long hoge_var = 0xdeadbeef;

// Ooops!
// We must define buf here as a global variable not as a local variable in
// print_hoge_var. Otherwise, the shared object depends on libc.so. I don't know
// the reason.
char buf[9];

void print_hex(unsigned long var) {
  // Simple itoa
  for (int i = 0; i < 8; i++) {
    char c = (var >> (4 * (7 - i))) & (0xf);
    c += (c < 10) ? '0' : 'a' - 10;
    buf[i] = c;
  }
  buf[8] = 0;

  RAW_PRINT_STR("print_hex: ");
  RAW_PRINT_STR(buf);
  RAW_PRINT_STR("\n");
}

void print_hoge_var() {
  // Simple itoa
  for (int i = 0; i < 8; i++) {
    char c = (hoge_var >> (4 * (7 - i))) & (0xf);
    c += (c < 10) ? '0' : 'a' - 10;
    buf[i] = c;
  }
  buf[8] = 0;

  RAW_PRINT_STR("====== print_hoge_var start ======\n");
  print_hex(hoge_var);
  print_hex((unsigned long)(&hoge_var));
  RAW_PRINT_STR("====== print_hoge_var end ======\n");
}
