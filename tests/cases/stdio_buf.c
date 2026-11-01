/* Buffering, C17 7.21.3p3 and 7.21.5.5–7.21.5.6: when a stream's bytes
 * reach its file, in each mode, seen from a second descriptor's
 * lseek(SEEK_END). Where the standard leaves the moment to the
 * implementation (a full buffer, a write too big for it, the default
 * size), glibc's choice is the reference (docs/LEDGER.md). Reports
 * through write(2) (t.h); the files are compared by the harness. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "t.h"

static int probe = -1;
static long sz(void) { return (long)lseek(probe, 0, SEEK_END); }
static FILE *opn(const char *name) {
    FILE *f = fopen(name, "w");
    probe = open(name, O_RDONLY);
    return f;
}
static void cls(FILE *f) {
    fclose(f);
    close(probe);
}
static char zs[20000];

int main(void) {
    memset(zs, 'z', sizeof zs);
    static char b16[16], b256[256], lb[16], sb[BUFSIZ];
    FILE *f;

    f = opn("full16.txt");
    t_kv("full16.setvbuf", setvbuf(f, b16, _IOFBF, 16));
    for (int i = 0; i < 16; i++)
        fputc('a', f);
    t_kv("full16.after16", sz());
    fputc('b', f);
    t_kv("full16.after17", sz());
    for (int i = 0; i < 15; i++)
        fputc('c', f);
    t_kv("full16.after32", sz());
    fputc('d', f);
    t_kv("full16.after33", sz());
    t_kv("full16.fflush", fflush(f));
    t_kv("full16.flushed", sz());
    fwrite(zs, 1, 40, f);
    t_kv("full16.fwrite40", sz());
    fwrite(zs, 1, 5, f);
    t_kv("full16.fwrite5", sz());
    fwrite(zs, 1, 12, f);
    t_kv("full16.fwrite12", sz());
    fputs("ABCDEFGHIJKLMNOPQRST", f);
    t_kv("full16.fputs20", sz());
    fwrite(zs, 1, 16, f);
    t_kv("full16.fwrite16_exact", sz());
    fputc('x', f);
    t_kv("full16.putc_after_exact", sz());
    cls(f);

    f = opn("full256.txt");
    setvbuf(f, b256, _IOFBF, 256);
    fwrite(zs, 1, 10, f);
    t_kv("full256.w10", sz());
    fwrite(zs, 1, 600, f);
    t_kv("full256.w600", sz());
    fflush(f);
    fwrite(zs, 1, 256, f);
    t_kv("full256.w256", sz());
    fputc('x', f);
    t_kv("full256.putc", sz());
    fflush(f);
    fwrite(zs, 1, 255, f);
    fwrite(zs, 1, 1, f);
    t_kv("full256.w255_w1", sz());
    fwrite(zs, 1, 1, f);
    t_kv("full256.w1_over", sz());
    fflush(f);
    fwrite(zs, 1, 1000, f);
    t_kv("full256.w1000", sz());
    fflush(f);
    fwrite(zs, 1, 100, f);
    fwrite(zs, 1, 1000, f);
    t_kv("full256.w100_w1000", sz());
    cls(f);

    f = opn("default.txt");
    for (int i = 0; i < 5000; i++)
        fputc('x', f);
    t_kv("default.putc5000", sz());
    fwrite(zs, 1, 9000, f);
    t_kv("default.fwrite9000", sz());
    fputs(zs + 19000, f);
    t_kv("default.fputs1000", sz());
    cls(f);

    f = opn("line16.txt");
    t_kv("line16.setvbuf", setvbuf(f, lb, _IOLBF, 16));
    fputc('a', f);
    t_kv("line16.putc", sz());
    fputs("bc", f);
    t_kv("line16.fputs", sz());
    fputs("d\ne", f);
    t_kv("line16.fputs_nl", sz());
    fputc('\n', f);
    t_kv("line16.putc_nl", sz());
    fputs("0123456789abcdefXYZ", f);
    t_kv("line16.fputs19", sz());
    cls(f);

    f = opn("line.txt");
    setvbuf(f, NULL, _IOLBF, 0);
    fputc('a', f);
    t_kv("line.putc", sz());
    fputs("bc", f);
    t_kv("line.fputs", sz());
    fputs("x\ny", f);
    t_kv("line.fputs_nl", sz());
    fputs("q\n", f);
    t_kv("line.fputs_q_nl", sz());
    cls(f);

    f = opn("none.txt");
    setbuf(f, NULL);
    fputs("abc", f);
    t_kv("none.fputs", sz());
    fputc('d', f);
    t_kv("none.putc", sz());
    fwrite(zs, 1, 3000, f);
    t_kv("none.fwrite", sz());
    cls(f);

    f = opn("setbuf.txt");
    setbuf(f, sb);
    for (int i = 0; i < 9000; i++)
        fputc('t', f);
    t_kv("setbuf.putc9000", sz());
    cls(f);

    FILE *g = fopen("g1.txt", "w");
    FILE *h = fopen("g2.txt", "w");
    fputs("one", g);
    fputs("two", h);
    t_kv("fflush.null", fflush(NULL));
    int p1 = open("g1.txt", O_RDONLY), p2 = open("g2.txt", O_RDONLY);
    t_kv("fflush.null.g1", (long)lseek(p1, 0, SEEK_END));
    t_kv("fflush.null.g2", (long)lseek(p2, 0, SEEK_END));
    close(p1);
    close(p2);
    fclose(g);
    fclose(h);

    f = fopen("/dev/full", "w");
    fputs("x", f);
    errno = 0;
    t_kv("full.fflush", fflush(f));
    t_kv("full.fflush_is_ENOSPC", errno == ENOSPC);
    t_kv("full.ferror", ferror(f) != 0);
    errno = 0;
    t_kv("full.fclose", fclose(f));
    t_kv("full.fclose_errno", errno);
    f = fopen("/dev/full", "w");
    setvbuf(f, NULL, _IONBF, 0);
    errno = 0;
    t_kv("full.unbuffered_fputc", fputc('y', f));
    t_kv("full.unbuffered_is_ENOSPC", errno == ENOSPC);
    fclose(f);
    errno = 0;
    t_kv("setvbuf.badmode", setvbuf(stderr, NULL, 7, 0) != 0);
    t_kv("setvbuf.badmode_errno", errno);
    return 0;
}
