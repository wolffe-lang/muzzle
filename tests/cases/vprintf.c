/* The va_list forms, C17 7.21.6.8–7.21.6.13: vprintf, vfprintf, vsprintf
 * and vsnprintf, fed by C's own va_start (<stdarg.h>). Enough arguments
 * to spill from registers to the stack, a list forwarded twice through
 * va_copy, the caller's list still usable afterwards (the callee works
 * on its own copy: 7.21.6.8's footnote leaves the caller's list
 * indeterminate, so only the copy is read again), %n through a list,
 * and vfprintf to a file. */
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#define _POSIX_C_SOURCE 200809L
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int out(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vprintf(fmt, ap);
    va_end(ap);
    return r;
}

static int twice(const char *fmt, ...) {
    va_list ap, cp;
    va_start(ap, fmt);
    va_copy(cp, ap);
    int r = vprintf(fmt, ap);
    r += vprintf(fmt, cp);
    va_end(cp);
    va_end(ap);
    return r;
}

static int to_buffer(char *b, size_t n, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(b, n, fmt, ap);
    va_end(ap);
    return r;
}

static int to_unbounded(char *b, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsprintf(b, fmt, ap);
    va_end(ap);
    return r;
}

static int to_file(FILE *f, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vfprintf(f, fmt, ap);
    va_end(ap);
    return r;
}

int main(void) {
    int r = out("%d %d %d %d %d %d %d %d %d %d|%s %s %s %s %s %s %s %s|%c%c%c%c%c%c%c%c%c%c\n",
                1, 2, 3, 4, 5, 6, 7, 8, 9, 10, "a", "b", "c", "d", "e", "f", "g", "h",
                'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't');
    printf("out.r=%d\n", r);
    r = out("%ld %lld %zu %jd %td %hhd %hd %lu %lx %llo\n", -1L, -2LL, (size_t)3, (intmax_t)-4,
            (ptrdiff_t)5, 300, 70000, 18446744073709551615UL, 0xabcdefUL, 0777ULL);
    printf("out.lengths.r=%d\n", r);
    r = twice("[%s:%d]", "twice", 2);
    printf("\ntwice.r=%d\n", r);
    char b[32];
    memset(b, 'X', sizeof b);
    r = to_buffer(b, 8, "%s-%d", "trunc", 12345);
    printf("vsnprintf.r=%d [%s] after=%c\n", r, b, b[9]);
    r = to_buffer(NULL, 0, "%*d", 20, 1);
    printf("vsnprintf.null.r=%d\n", r);
    r = to_unbounded(b, "%#x|%o|%5.2s", 255, 8, "abc");
    printf("vsprintf.r=%d [%s]\n", r, b);
    int n = 0;
    r = out("abc%n%d\n", &n, 9);
    printf("n.via_list=%d r=%d\n", n, r);
    FILE *f = fopen("v.txt", "w");
    r = to_file(f, "%s %d %p\n", "file", 42, (void *)0);
    printf("vfprintf.r=%d\n", r);
    r = to_file(f, "%-10s|%+d\n", "left", 7);
    printf("vfprintf.r2=%d\n", r);
    fclose(f);
    r = to_file(stdout, "%s\n", "vfprintf to stdout");
    printf("vfprintf.stdout.r=%d\n", r);
    return 0;
}
