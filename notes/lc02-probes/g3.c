/* g3: glibc as a black box, round 3. Line buffering in detail, the
 * direct-write rule for large buffers, fputs's return, a failed printf
 * after part of its output, unknown conversions with flags, and fflush
 * of an input stream that cannot seek. Observed only through a second
 * descriptor's lseek(SEEK_END) and returns. */
#define _POSIX_C_SOURCE 200809L
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wformat-invalid-specifier"
#pragma clang diagnostic ignored "-Wformat-extra-args"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

static int pfd = -1;
static long sz(void) { return (long)lseek(pfd, 0, SEEK_END); }
static void kv(const char *k, long v) { printf("%s=%ld\n", k, v); }
static FILE *opn(const char *name) { FILE *f = fopen(name, "w"); pfd = open(name, O_RDONLY); return f; }
static void cls(FILE *f) { fclose(f); close(pfd); }
static char big[20000];

int main(void) {
    memset(big, 'z', sizeof big);
    static char b16[16], b256[256];
    FILE *f;
    /* line buffered, 16-byte user buffer, then a NULL buffer of BUFSIZ */
    f = opn("l1.out"); setvbuf(f, b16, _IOLBF, 16);
    fputc('a', f); kv("l16.putc_a", sz()); fputc('b', f); kv("l16.putc_b", sz());
    fputs("cd", f); kv("l16.fputs_cd", sz()); fprintf(f, "%s", "ef"); kv("l16.fprintf_ef", sz());
    fputc('\n', f); kv("l16.putc_nl", sz()); fwrite("gh", 1, 2, f); kv("l16.fwrite_gh", sz());
    cls(f);
    f = opn("l2.out"); setvbuf(f, NULL, _IOLBF, 0);
    fputc('a', f); kv("lnull.putc_a", sz()); fputs("bc", f); kv("lnull.fputs_bc", sz());
    fprintf(f, "%d", 12); kv("lnull.fprintf", sz()); fputs("x\ny", f); kv("lnull.fputs_nl", sz());
    fputc('z', f); kv("lnull.putc_z", sz()); fputs("q\n", f); kv("lnull.fputs_q_nl", sz());
    fwrite(big, 1, 5000, f); kv("lnull.fwrite5000", sz()); fputc('\n', f); kv("lnull.nl", sz());
    cls(f);
    f = opn("l3.out"); setvbuf(f, NULL, _IOLBF, 64);
    fputs("ab", f); kv("l64.fputs_ab", sz()); fputs("c\nd", f); kv("l64.fputs_nl", sz());
    for (int i = 0; i < 70; i++) fputc('w', f);
    kv("l64.after70", sz()); cls(f);
    /* fully buffered, 256-byte user buffer: the direct-write rule */
    f = opn("f1.out"); setvbuf(f, b256, _IOFBF, 256);
    fwrite(big, 1, 10, f); kv("f256.w10", sz()); fwrite(big, 1, 600, f); kv("f256.w600", sz());
    fflush(f); kv("f256.flush", sz());
    fwrite(big, 1, 256, f); kv("f256.w256_exact", sz()); fputc('x', f); kv("f256.putc", sz());
    fflush(f); fwrite(big, 1, 255, f); kv("f256.w255", sz()); fwrite(big, 1, 1, f); kv("f256.w1_fill", sz());
    fwrite(big, 1, 1, f); kv("f256.w1_over", sz());
    fflush(f); fwrite(big, 1, 300, f); kv("f256.w300_empty", sz()); fflush(f); kv("f256.flush2", sz());
    fwrite(big, 1, 1000, f); kv("f256.w1000_empty", sz()); fflush(f);
    fwrite(big, 1, 100, f); fwrite(big, 1, 1000, f); kv("f256.w100_w1000", sz());
    cls(f);
    f = opn("f2.out");
    fwrite(big, 1, 5000, f); kv("fdef.w5000", sz()); fwrite(big, 1, 100, f); kv("fdef.w100", sz());
    fwrite(big, 1, 9000, f); kv("fdef.w9000", sz()); fflush(f); kv("fdef.flush", sz());
    fputs(big + 19000, f); kv("fdef.fputs1000", sz());
    cls(f);
    f = opn("f3.out"); setvbuf(f, b16, _IOFBF, 16);
    fwrite(big, 1, 16, f); kv("f16.w16_exact", sz()); fputc('x', f); kv("f16.putc", sz());
    cls(f);
    /* fputs's value */
    f = opn("r1.out");
    kv("fputs.1", fputs("a", f)); kv("fputs.100", fputs(big + 19900, f)); kv("fputs.empty", fputs("", f));
    setvbuf(stderr, NULL, _IONBF, 0);
    kv("fputs.stderr", fputs("e", stderr)); kv("fputs.stdout", fputs("o\n", stdout));
    cls(f);
    /* a printf that fails after part of its output */
    f = opn("p1.out");
    errno = 0; kv("p.eilseq", fprintf(f, "abc%lcdef", (wint_t)0xe9)); kv("p.eilseq.errno", errno);
    fflush(f); kv("p.eilseq.size", sz());
    errno = 0; kv("p.einval", fprintf(f, "abc%")); kv("p.einval.errno", errno); fflush(f); kv("p.einval.size", sz());
    errno = 0; kv("p.big_then_bad", fprintf(f, "%5000s%", "x")); kv("p.big_then_bad.errno", errno);
    fflush(f); kv("p.big_then_bad.size", sz());
    cls(f);
    char sb[16]; memset(sb, 'X', sizeof sb); sb[15] = 0;
    errno = 0; int r = snprintf(sb, 16, "abc%lcdef", (wint_t)0xe9);
    printf("snp.eilseq r=%d e=%d sb=[%s]\n", r, errno, sb);
    memset(sb, 'X', sizeof sb); sb[15] = 0;
    errno = 0; r = snprintf(sb, 16, "abc%"); printf("snp.einval r=%d e=%d sb=[%s]\n", r, errno, sb);
    memset(sb, 'X', sizeof sb); sb[15] = 0;
    errno = 0; r = sprintf(sb, "ab%"); printf("sp.einval r=%d e=%d sb=[%s]\n", r, errno, sb);
    /* unknown conversions with flags, width, precision and length */
    printf("u1 [%5k] [%-5k] [%.3k] [%#k] [%05k] [%hhk] [%+ k] [%5.2lk]\n");
    printf("u2 [%*k] [%.*k]\n", 7, 3);
    printf("u3 [%5K] [%1$k]\n");
    /* fflush of an input stream that cannot seek: stdin is a pipe here when run by the harness */
    errno = 0; kv("fflush.stdin", fflush(stdin)); kv("fflush.stdin.errno", errno);
    int p[2]; (void)p;
    /* fflush of a read stream at a file: the descriptor's offset afterwards */
    f = fopen("l1.out", "w"); fputs("0123456789", f); fclose(f);
    f = fopen("l1.out", "r"); fgetc(f); fgetc(f); kv("rd.offset_before", (long)lseek(fileno(f), 0, SEEK_CUR));
    kv("rd.fflush", fflush(f)); kv("rd.offset_after", (long)lseek(fileno(f), 0, SEEK_CUR));
    ungetc('Q', f); kv("rd.fflush_unget", fflush(f)); kv("rd.offset_unget", (long)lseek(fileno(f), 0, SEEK_CUR));
    kv("rd.next", fgetc(f));
    fclose(f);
    /* fgets at the edges */
    f = fopen("l1.out", "r"); char g[4];
    char *gr = fgets(g, 4, f); printf("fgets4 %d [%s]\n", gr == g, g);
    while (fgets(g, 4, f)) {}
    memcpy(g, "kkk", 4); gr = fgets(g, 4, f); printf("fgets.eof %d [%s]\n", gr == NULL, g);
    fclose(f);
    /* fdopen modes */
    int fd = open("l1.out", O_RDONLY);
    errno = 0; f = fdopen(fd, "w"); kv("fdopen.w_on_rdonly", f != NULL); kv("fdopen.errno", errno);
    if (f) fclose(f); else close(fd);
    fd = open("l1.out", O_RDONLY); errno = 0; f = fdopen(fd, "q"); kv("fdopen.badmode", f != NULL); kv("fdopen.badmode.errno", errno);
    if (f) fclose(f); else close(fd);
    errno = 0; f = fdopen(99, "r"); kv("fdopen.badfd", f != NULL); kv("fdopen.badfd.errno", errno);
    fd = open("l1.out", O_RDONLY); f = fdopen(fd, "r"); kv("fdopen.r.getc", fgetc(f)); kv("fdopen.fclose", fclose(f));
    errno = 0; kv("fdopen.closed_fd", (long)lseek(fd, 0, SEEK_CUR)); kv("fdopen.closed_fd.errno", errno);
    /* setvbuf's size argument with a NULL buffer, and setbuf */
    f = opn("s1.out"); setvbuf(f, NULL, _IOFBF, 100);
    for (int i = 0; i < 150; i++) fputc('s', f);
    kv("sv100.after150", sz()); cls(f);
    f = opn("s2.out"); static char sbuf[BUFSIZ]; setbuf(f, sbuf);
    for (int i = 0; i < 9000; i++) fputc('t', f);
    kv("setbuf.after9000", sz()); cls(f);
    return 0;
}
