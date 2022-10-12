#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "raw_write.h"

int main() {
    RAW_DEBUG_MESSAGE();
    RAW_NOP4();
    write(1, "Hello using write(2)\n", 21);
    RAW_DEBUG_MESSAGE();
    printf("Hello using printf(3)\n");

    const char str[] = "http://www.tutorialspoint.com";
    const char ch = '.';
    char* ret;

    ret = strchr(str, ch);

    printf("String after |%c| is - |%s|\n", ch, ret);

    fwrite("hogeh\n", sizeof(char), 6, stdout);
    return 0;
}
