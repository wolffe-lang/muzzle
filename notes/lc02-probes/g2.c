#define _POSIX_C_SOURCE 200809L
/* g2: glibc as a black box, the stream behaviours an implementation
 * chooses: when a buffer reaches the file, what the nonnegative returns
 * are, pushback depth, errors and errno. Observed through a second
 * descriptor's lseek(SEEK_END), never through glibc's internals. */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int probe_fd = -1;
static long size_now(void) { return (long)lseek(probe_fd, 0, SEEK_END); }
static void kv(const char *k, long v) { printf("%s=%ld\n", k, v); }

int main(void) {
    FILE *f = fopen("g2.out", "w");
    probe_fd = open("g2.out", O_RDONLY);
    static char ub[16];
    kv("setvbuf16", setvbuf(f, ub, _IOFBF, 16));
    for (int i = 0; i < 16; i++) fputc('a', f);
    kv("full16.after16", size_now());
    fputc('b', f); kv("full16.after17", size_now());
    for (int i = 0; i < 14; i++) fputc('c', f);
    kv("full16.after31", size_now());
    fputc('d', f); kv("full16.after32", size_now());
    fputc('e', f); kv("full16.after33", size_now());
    fflush(f); kv("full16.flushed", size_now());
    kv("fwrite40", (long)fwrite("0123456789012345678901234567890123456789", 1, 40, f));
    kv("full16.after_fwrite40", size_now());
    kv("fwrite5", (long)fwrite("abcde", 1, 5, f)); kv("full16.after_fwrite5", size_now());
    kv("fwrite12", (long)fwrite("abcdefghijkl", 1, 12, f)); kv("full16.after_fwrite12", size_now());
    kv("fputs20", fputs("ABCDEFGHIJKLMNOPQRST", f)); kv("full16.after_fputs20", size_now());
    fclose(f); kv("closed", size_now()); close(probe_fd);

    f = fopen("g2b.out", "w"); probe_fd = open("g2b.out", O_RDONLY);
    static char lb[16];
    kv("setvbuf_lbf", setvbuf(f, lb, _IOLBF, 16));
    fputs("ab", f); kv("lbf.after_ab", size_now());
    fputs("c\nd", f); kv("lbf.after_nl", size_now());
    fputs("0123456789abcdefXYZ", f); kv("lbf.after_19", size_now());
    fclose(f); close(probe_fd);

    f = fopen("g2c.out", "w"); probe_fd = open("g2c.out", O_RDONLY);
    for (int i = 0; i < 5000; i++) fputc('x', f);
    kv("default.after5000", size_now());
    for (int i = 0; i < 5000; i++) fputc('y', f);
    kv("default.after10000", size_now());
    fclose(f); close(probe_fd);

    f = fopen("g2d.out", "w"); probe_fd = open("g2d.out", O_RDONLY);
    setbuf(f, NULL);
    fputs("abc", f); kv("nbf.after_abc", size_now());
    fputc('d', f); kv("nbf.after_d", size_now());
    kv("nbf.fprintf", fprintf(f, "%d-%s", 42, "zz")); kv("nbf.after_fprintf", size_now());
    fclose(f); close(probe_fd);

    f = fopen("g2e.out", "w");
    kv("ret.fputs", fputs("abc", f));
    kv("ret.fputs_empty", fputs("", f));
    kv("ret.fputc", fputc(0x1e9, f));
    kv("ret.fwrite0size", (long)fwrite("a", 0, 5, f));
    kv("ret.fwrite0n", (long)fwrite("a", 5, 0, f));
    kv("ret.fclose", fclose(f));
    kv("ret.puts", puts("puts-line"));
    kv("ret.puts_empty", puts(""));
    kv("ret.putchar", putchar('Q')); putchar('\n');

    f = fopen("g2e.out", "r");
    kv("unget.first", fgetc(f));
    kv("unget.push1", ungetc('Z', f));
    kv("unget.push2", ungetc('Y', f));
    kv("unget.read1", fgetc(f));
    kv("unget.read2", fgetc(f));
    kv("unget.after", fgetc(f));
    kv("unget.eof_push", ungetc(EOF, f));
    char line[8];
    kv("fgets1", fgets(line, 1, f) == line); kv("fgets1.len", (long)strlen(line));
    char *got = fgets(line, 0, f); kv("fgets0.null", got == NULL);
    errno = 0; kv("fputc_on_r", fputc('x', f)); kv("fputc_on_r.errno", errno); kv("fputc_on_r.ferror", ferror(f) != 0);
    clearerr(f); kv("cleared", ferror(f));
    while (fgetc(f) != EOF) {}
    kv("eof", feof(f) != 0);
    kv("eof.ungetc", ungetc('k', f)); kv("eof.after_ungetc", feof(f) != 0); kv("eof.read_k", fgetc(f));
    kv("eof.again", fgetc(f));
    fclose(f);

    f = fopen("g2e.out", "w");
    errno = 0; kv("fgetc_on_w", fgetc(f)); kv("fgetc_on_w.errno", errno); kv("fgetc_on_w.ferror", ferror(f) != 0);
    kv("fgetc_on_w.feof", feof(f) != 0);
    fclose(f);

    errno = 0; f = fopen("g2e.out", "z"); kv("badmode.null", f == NULL); kv("badmode.errno", errno);
    errno = 0; f = fopen("g2e.out", "rw"); kv("mode_rw.null", f == NULL); if (f) fclose(f);
    errno = 0; f = fopen("g2e.out", "r+bx"); kv("mode_r+bx.null", f == NULL); kv("mode_r+bx.errno", errno); if (f) fclose(f);
    errno = 0; f = fopen("no/such", "r"); kv("enoent.null", f == NULL); kv("enoent.errno", errno);
    errno = 0; f = fopen("/dev/full", "w"); fputs("x", f);
    kv("full.fflush", fflush(f)); kv("full.errno", errno); kv("full.ferror", ferror(f) != 0);
    errno = 0; kv("full.fclose", fclose(f)); kv("full.fclose_errno", errno);

    f = fopen("g2f.out", "w"); fputs("abcdef", f); fclose(f);
    f = fopen("g2f.out", "r+");
    kv("rplus.get", fgetc(f)); kv("rplus.flush", fflush(f)); kv("rplus.put", fputc('X', f)); fclose(f);
    f = fopen("g2f.out", "r"); fgets(line, 8, f); printf("rplus.file=%s\n", line); fclose(f);
    f = fopen("g2f.out", "a"); fputs("gh", f); fclose(f);
    f = fopen("g2f.out", "a+"); kv("aplus.get", fgetc(f)); fclose(f);
    f = fopen("g2f.out", "r"); char l2[16]; fgets(l2, 16, f); printf("append.file=%s\n", l2); fclose(f);

    errno = 0; kv("isatty1", isatty(1)); kv("isatty1.errno", errno);
    errno = 0; kv("isatty99", isatty(99)); kv("isatty99.errno", errno);
    kv("fileno", fileno(stdin) * 100 + fileno(stdout) * 10 + fileno(stderr));
    errno = 0; kv("setvbuf_bad", setvbuf(stderr, NULL, 7, 0)); kv("setvbuf_bad.errno", errno);
    f = fopen("g2e.out", "r");
    char rb[8]; size_t got2 = fread(rb, 3, 5, f); kv("fread.short", (long)got2); kv("fread.eof", feof(f) != 0);
    fclose(f);
    printf("buffered-then-exit");
    return 0;
}
