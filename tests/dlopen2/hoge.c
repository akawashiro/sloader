#include <stdio.h>

void fromMain();

void hoge() {
    printf("hogehoge\n");
    fromMain();
}
