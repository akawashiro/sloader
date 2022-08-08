#include <stdio.h>
#include <stdlib.h>

__thread int tls_i = 10;

static void before_main(void) __attribute__((constructor));
static void after_main(void) __attribute__((destructor));

static void before_main(void) {
    puts("before_main");
}

static void after_main(void) {
    puts("after_main");
}

void bye(void) {
    printf("That was all, folks\n");
}

int main() {
    puts("Hello from libc!");
    printf("tls_i = %d\n", tls_i);

    if (atexit(bye) != 0) {
        fprintf(stderr, "cannot set exit function\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
    return 0;
}
