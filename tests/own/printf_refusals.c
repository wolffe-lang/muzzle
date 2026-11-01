/* muzzle's own (R-lc02-1 = B): the floating conversions f F e E g G a A
 * belong to lc03, so a printf-family call that meets one answers -1 with
 * errno EINVAL, as any failed call does: a stream receives none of its
 * output; in memory what came before the conversion stays, with its null.
 * Positional arguments (%n$, *m$) are refused the same way: not provided
 * yet. glibc prints all of these, so this case checks itself against
 * muzzle alone (tools/difftest's own cases): a line "ok NAME" per check,
 * exit 0 only if every check holds. */
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wformat-extra-args"
#pragma clang diagnostic ignored "-Wformat-invalid-specifier"
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int failed;

static void check(const char *name, int cond) {
    char line[96];
    int n = snprintf(line, sizeof line, "%s %s\n", cond ? "ok" : "FAIL", name);
    write(1, line, (size_t)n);
    if (!cond)
        failed = 1;
}

/* snprintf of fmt with one double: -1, EINVAL, and the buffer holds what
 * came before the conversion (want), with its null. */
static void refused_double(const char *name, const char *fmt, const char *want) {
    char b[32];
    memset(b, 'k', sizeof b);
    errno = 0;
    int r = snprintf(b, sizeof b, fmt, 1.5);
    check(name, r == -1 && errno == EINVAL && strcmp(b, want) == 0);
}

int main(void) {
    refused_double("f", "x%fy", "x");
    refused_double("F", "x%Fy", "x");
    refused_double("e", "x%ey", "x");
    refused_double("E", "x%Ey", "x");
    refused_double("g", "x%gy", "x");
    refused_double("G", "x%Gy", "x");
    refused_double("a", "x%ay", "x");
    refused_double("A", "x%Ay", "x");
    refused_double("lf", "%lf", "");
    refused_double("10.3f", "%10.3f", "");
    refused_double("-+#0e", "%-+#0e", "");
    char b[32];
    errno = 0;
    int r = snprintf(b, sizeof b, "%Lf", 1.5L);
    check("Lf", r == -1 && errno == EINVAL && b[0] == 0);
    errno = 0;
    r = snprintf(b, sizeof b, "%d %f", 7, 2.0);
    check("after_an_int", r == -1 && errno == EINVAL && strcmp(b, "7 ") == 0);
    errno = 0;
    r = snprintf(NULL, 0, "%f", 2.0);
    check("null_buffer", r == -1 && errno == EINVAL);
    errno = 0;
    r = snprintf(b, sizeof b, "%1$d", 5);
    check("positional", r == -1 && errno == EINVAL && b[0] == 0);
    errno = 0;
    r = snprintf(b, sizeof b, "%*1$d", 5);
    check("positional_star", r == -1 && errno == EINVAL);

    /* to a stream: nothing of the call reaches the file */
    FILE *f = fopen("refused.txt", "w");
    fputs("before|", f);
    errno = 0;
    r = fprintf(f, "abc%fdef", 3.0);
    check("stream.r", r == -1 && errno == EINVAL);
    fputs("|after", f);
    fclose(f);
    int fd = open("refused.txt", O_RDONLY);
    char got[32];
    memset(got, 0, sizeof got);
    read(fd, got, sizeof got - 1);
    close(fd);
    check("stream.file", strcmp(got, "before||after") == 0);

    /* unbuffered: nothing reaches the descriptor */
    f = fopen("refused2.txt", "w");
    setvbuf(f, NULL, _IONBF, 0);
    errno = 0;
    r = fprintf(f, "abc%g", 3.0);
    check("unbuffered.r", r == -1 && errno == EINVAL);
    fclose(f);
    fd = open("refused2.txt", O_RDONLY);
    check("unbuffered.empty", lseek(fd, 0, SEEK_END) == 0);
    close(fd);
    return failed;
}
