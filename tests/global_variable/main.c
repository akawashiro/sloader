extern int hoge_var;

void print_hoge_var();
void print_hex(unsigned long var);

int main() {
  print_hex(hoge_var);
  print_hex(&hoge_var); // This address must be the same with print_hoge_var.

  print_hoge_var(); // Should be 0xdeadbeef
  hoge_var = 0xabcdef12;
  print_hoge_var();
  hoge_var = 0xaabbccdd;
  print_hoge_var();
  return 0;
}
