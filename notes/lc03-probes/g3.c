/* lc03 g3: glibc's first perror on an unused stderr, by entry errno,
 * each in a forked child whose stderr nothing has touched (black box). */
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void report(const char *what, int before) {
    int after = errno;
    char b[96];
    int n = snprintf(b, sizeof b, "%s before=%d after=%d\n", what, before, after);
    write(1, b, (size_t)n);
}

static void child(int scenario) {
    pid_t p = fork();
    if (p == 0) {
        switch (scenario) {
        case 0: errno = 0; perror("a"); report("fresh perror errno0", 0); break;
        case 1: errno = ENOENT; perror("b"); report("fresh perror ENOENT", ENOENT); break;
        case 2: errno = 0; perror(NULL); report("fresh perror(NULL) errno0", 0); break;
        case 3: errno = 0; perror(""); report("fresh perror(\"\") errno0", 0); break;
        case 4: fputs("x\n", stderr); errno = 0; perror("c"); report("used stderr, perror errno0", 0); break;
        case 5: errno = 0; fputs("y\n", stderr); report("fresh fputs errno0", 0); break;
        case 6: errno = 0; fprintf(stderr, "z\n"); report("fresh fprintf errno0", 0); break;
        case 7: errno = 0; fflush(stderr); perror("d"); report("flushed stderr, perror errno0", 0); break;
        case 8: errno = 0; setvbuf(stderr, NULL, _IOLBF, 0); perror("e"); report("setvbuf stderr, perror errno0", 0); break;
        }
        _exit(0);
    }
    int st;
    waitpid(p, &st, 0);
}

int main(void) {
    for (int s = 0; s <= 8; s++)
        child(s);
    return 0;
}
