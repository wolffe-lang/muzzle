/* C17 7.22.4.5 _Exit: no atexit handler runs, and (glibc's choice, and
 * muzzle's) no stream is flushed. Only the direct write reaches stdout. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void h(void) { write(1, "handler\n", 8); }

int main(void) {
    atexit(h);
    printf("buffered, never flushed\n");
    write(1, "direct\n", 7);
    _Exit(5);
}
