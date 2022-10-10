#include <stdio.h>

int main(int argc, char* argv[]) {
    int i;
    printf("%d\n", argc);
    for (i = 1; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    return 0;
}
