#include <stdio.h>
#include <unistd.h>

#include "raw_write.h"

int main() {
    RAW_DEBUG_MESSAGE();
    RAW_NOP4();
    write(1, "Hello using write(2)\n", 21);
    RAW_DEBUG_MESSAGE();
    printf("Hello using printf(3)\n");
    return 0;
}
