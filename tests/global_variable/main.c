extern int hoge_var;

int hoge(int a, int b);
void print_hoge_var();

int main() {
    hoge_var = 123;
    print_hoge_var();
    hoge_var = 456;
    print_hoge_var();
    return hoge(2, 40);
}
