/* selftest control: run with stdout on a pseudo-terminal. */
// difftest: tty
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
int main(void) {
    printf("tty=%d\n", isatty(1));
    return 0;
}
