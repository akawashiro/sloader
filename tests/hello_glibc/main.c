#include <stdio.h>

#include "/home/akira/sloader/raw_write.h"

int main() {
    RAW_DEBUG_MESSAGE();
    RAW_NOP4();
    write(1, "Hello using write(2)\n", 21);
    RAW_DEBUG_MESSAGE();
    printf("Hello using printf(1)\n");
    return 0;
}
