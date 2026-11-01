/* stderr is unbuffered (C17 7.21.3p7: "not fully buffered"; glibc and
 * muzzle send each call's bytes at once): its bytes interleave with
 * direct write(2)s to fd 2 in call order. stdout, redirected to a file,
 * is fully buffered and arrives at exit. setvbuf can buffer stderr (a
 * 256-byte buffer: under 128 bytes a first multi-byte write goes out at
 * once, docs/LEDGER.md). The harness compares stdout and stderr. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void mark(int fd, const char *m) { write(fd, m, strlen(m)); }

int main(void) {
    fputs("e1", stderr);
    mark(2, "[1]");
    fprintf(stderr, "e%d", 2);
    mark(2, "[2]");
    fputc('3', stderr);
    mark(2, "[3]");
    fwrite("e4\n", 1, 3, stderr);
    mark(2, "[4]");
    printf("o1");
    mark(1, "[5]");
    fprintf(stderr, "%s=%d\n", "isatty2", isatty(2));
    static char eb[256];
    setvbuf(stderr, eb, _IOFBF, sizeof eb);
    fputs("buffered", stderr);
    mark(2, "[6]");
    fflush(stderr);
    mark(2, "[7]");
    fputs("at-exit", stderr);
    printf("o2\n");
    return 0;
}
