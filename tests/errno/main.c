#include <errno.h>
#include <stdio.h>
#include <string.h>

int main() {
    FILE* pf;
    int errnum;
    pf = fopen("unexist.txt", "rb");
    errnum = errno;

    printf("errno: %s\n", strerror(errnum));
    return 0;
}
