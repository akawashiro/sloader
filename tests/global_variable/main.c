extern int hoge_var;

int hoge(int a, int b);
void print_hoge_var();

int main() {
    print_hoge_var();  // Should be 0xdeadbeef
    hoge_var = 0xabcdef12;
    print_hoge_var();
    hoge_var = 0xaabbccdd;
    print_hoge_var();
    return hoge(2, 40);
}
