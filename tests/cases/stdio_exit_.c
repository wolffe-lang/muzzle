/* _exit does not flush (POSIX XSH _exit; C17 7.22.4.5p2 leaves it to the
 * implementation, and glibc and muzzle do not): bytes still in a
 * stream's buffer are lost, in stdout and in a file. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    printf("lost: still in stdout's buffer");
    write(1, "[direct]\n", 9);
    FILE *f = fopen("lost.txt", "w");
    fputs("lost too", f);
    FILE *g = fopen("flushed.txt", "w");
    fputs("flushed before _exit", g);
    fflush(g);
    _exit(4);
}
