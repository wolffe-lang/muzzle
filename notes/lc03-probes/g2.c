/* lc03 g2: what glibc's perror does to errno (black box). Each line is
 * errno before and after, in a fresh process per scenario argument. */
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void show(const char *what, int before) {
    int after = errno;
    char b[96];
    int n = snprintf(b, sizeof b, "%s before=%d after=%d\n", what, before, after);
    write(1, b, (size_t)n);
}

int main(void) {
    errno = 0; perror("first, errno 0"); show("first0", 0);
    errno = 0; perror("second, errno 0"); show("second0", 0);
    errno = 0; perror(NULL); show("null0", 0);
    errno = 22; perror("einval"); show("einval", 22);
    errno = 1000; perror("unknown"); show("unknown", 1000);
    errno = -1; perror("negative"); show("negative", -1);
    errno = 0; fputs("plain\n", stderr); show("fputs0", 0);
    errno = 0; strerror(0); show("strerror0", 0);
    errno = 0; strerror(1000); show("strerror1000", 0);
    return 0;
}
