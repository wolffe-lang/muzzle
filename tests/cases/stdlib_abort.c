/* C17 7.22.4.1 abort: abnormal termination by SIGABRT (the harness sees
 * 128 + 6), no atexit handler, and (C17 leaves it open; glibc since 2.27
 * does not) no stream flushed: the buffered line never arrives, the
 * unbuffered stderr line and the direct write do. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void h(void) { write(1, "handler\n", 8); }

int main(void) {
    atexit(h);
    write(1, "before\n", 7);
    printf("buffered\n");
    fputs("stderr line\n", stderr);
    abort();
}
