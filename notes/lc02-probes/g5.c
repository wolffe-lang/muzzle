/* g5: glibc as a black box, round 4: the buffer size under which a write
 * that does not fit goes out whole, printf's flushes part way through a
 * long call, a line-buffered write with a newline inside a long write,
 * and how an unknown conversion is printed back (flag order). Observed
 * only through a second descriptor's lseek(SEEK_END) and output. */
#define _POSIX_C_SOURCE 200809L
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wformat-invalid-specifier"
#pragma clang diagnostic ignored "-Wformat-extra-args"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int pfd = -1;
static long sz(void) { return (long)lseek(pfd, 0, SEEK_END); }
static void kv(const char *k, long v) { printf("%s=%ld\n", k, v); }
static FILE *opn(const char *name) { FILE *f = fopen(name, "w"); pfd = open(name, O_RDONLY); return f; }
static void cls(FILE *f) { fclose(f); close(pfd); }
static char big[40000], ub[4096];

int main(void) {
    memset(big, 'z', sizeof big);
    /* the threshold: a set-up buffer of size s, 3*s+5 bytes after one putc */
    int sizes[] = {16, 32, 64, 100, 120, 126, 127, 128, 129, 130, 200, 255};
    for (unsigned i = 0; i < sizeof sizes / sizeof sizes[0]; i++) {
        int s = sizes[i];
        FILE *f = opn("t.out");
        setvbuf(f, ub, _IOFBF, (size_t)s);
        fputc('a', f);
        fwrite(big, 1, (size_t)(3 * s + 5), f);
        char k[32];
        snprintf(k, sizeof k, "thr.%d", s);
        kv(k, sz());
        cls(f);
    }
    /* printf part way: a fresh default stream, then one with 10 pending */
    int ns[] = {4095, 4096, 4097, 4098, 5000, 8191, 8192, 8193, 8194, 9000, 13000};
    for (unsigned i = 0; i < sizeof ns / sizeof ns[0]; i++) {
        FILE *f = opn("p.out");
        fprintf(f, "%*s", ns[i], "x");
        char k[32];
        snprintf(k, sizeof k, "pf.fresh.%d", ns[i]);
        kv(k, sz());
        cls(f);
        f = opn("p.out");
        fputs("0123456789", f);
        fprintf(f, "%*s", ns[i], "x");
        snprintf(k, sizeof k, "pf.after10.%d", ns[i]);
        kv(k, sz());
        cls(f);
    }
    /* a failed long printf: what of it reached the file */
    int fs[] = {4095, 4096, 4097, 9000};
    for (unsigned i = 0; i < sizeof fs / sizeof fs[0]; i++) {
        FILE *f = opn("e.out");
        fputs("ab", f);
        fflush(f);
        int r = fprintf(f, "%*s%", fs[i], "x");
        fflush(f);
        char k[32];
        snprintf(k, sizeof k, "pf.fail.%d", fs[i]);
        printf("%s=%ld r=%d\n", k, sz(), r);
        cls(f);
    }
    /* fputs and fwrite after a failed printf on a fresh stream */
    FILE *f = opn("w.out");
    fprintf(f, "abc%");
    fwrite(big, 1, 20, f);
    kv("after_failed.w20", sz());
    cls(f);
    /* line buffered, a long write with a newline inside */
    f = opn("l.out");
    setvbuf(f, NULL, _IOLBF, 0);
    fputc('s', f);
    big[4500] = '\n';
    fwrite(big, 1, 5000, f);
    kv("lbf.long_nl_4500", sz());
    big[4500] = 'z';
    cls(f);
    f = opn("l2.out");
    setvbuf(f, NULL, _IOLBF, 0);
    fputc('s', f);
    big[100] = '\n';
    fwrite(big, 1, 5000, f);
    kv("lbf.long_nl_100", sz());
    big[100] = 'z';
    cls(f);
    f = opn("l3.out");
    setvbuf(f, NULL, _IOLBF, 0);
    fputc('s', f);
    fprintf(f, "%5000s\n%s", "x", "tail");
    kv("lbf.printf_long_nl", sz());
    cls(f);
    /* unknown conversions printed back */
    printf("u1 [%-+ #0k] [%0#k] [%#0k] [% +k] [%+-k] [%-0k] [%0-k] [%'k] [%' k] [%'-k] [% k] [%0k]\n");
    printf("u2 [%*k] [%-*k] [%.*k] [%.*k] [%*.*k]\n", -7, 4, -1, 0, 3, 2);
    printf("u3 [%.k] [%.0k] [%10.k] [%-10.5k] [%lk] [%Lk] [%jk] [%%k] [%5%k]\n");
    printf("u4 [%#5.3K] [%+05y] [%5@] [%-3!]\n");
    return 0;
}
