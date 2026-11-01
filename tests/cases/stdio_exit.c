/* exit flushes every open stream (C17 7.22.4.4p4), and returning from
 * main is exit (5.1.2.2.3). stdout redirected to a file is fully
 * buffered (C17 7.21.3p7), so its bytes arrive after the direct write(2)s
 * made before exit. A stream opened and never closed still reaches its
 * file. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void mark(const char *m) { write(1, m, strlen(m)); }

static void leave(int status) {
    printf("leaving with %d", status);
    exit(status);
}

int main(void) {
    printf("first ");
    mark("[direct 1]");
    puts("a puts line");
    mark("[direct 2]");
    putchar('p');
    FILE *f = fopen("kept.txt", "w");
    fputs("written, never closed\n", f);
    FILE *g = fopen("closed.txt", "w");
    fputs("closed\n", g);
    fclose(g);
    FILE *h = fopen("unbuffered.txt", "w");
    setvbuf(h, NULL, _IONBF, 0);
    fputs("unbuffered\n", h);
    static char big[5000];
    memset(big, 'B', sizeof big);
    fwrite(big, 1, sizeof big, stdout);
    mark("[direct 3]");
    leave(5);
    return 1;
}
