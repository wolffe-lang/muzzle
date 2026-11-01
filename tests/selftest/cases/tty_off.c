/* selftest control: no marker, so stdout is the harness's file. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
int main(void) {
    printf("tty=%d\n", isatty(1));
    return 0;
}
