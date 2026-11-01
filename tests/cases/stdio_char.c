/* Character input/output, C17 7.21.7: fputc, putc, fputs, fgetc, getc,
 * getchar, fgets, and the indicators (7.21.10: feof, ferror, clearerr).
 * stdin is stdio_char.in. Reports through write(2) (t.h), never through
 * stdout's stream. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "t.h"

static void line(const char *k, const char *s) {
    t_str(k);
    t_str("=[");
    t_str(s);
    t_str("]\n");
}

int main(void) {
    FILE *f = fopen("c.txt", "w");
    t_kv("fputc.a", fputc('a', f));
    t_kv("fputc.0x1e9", fputc(0x1e9, f));
    t_kv("fputc.-1", fputc(-1, f));
    t_kv("putc.b", putc('b', f));
    t_kv("fputs.1", fputs("cd\n", f));
    t_kv("fputs.empty", fputs("", f));
    t_kv("fputs.long", fputs("a line longer than the eight bytes fgets will take\nend", f));
    t_kv("ferror.w", ferror(f));
    t_kv("feof.w", feof(f));
    errno = 0;
    t_kv("fgetc.on_w", fgetc(f));
    t_kv("fgetc.on_w_is_EBADF", errno == EBADF);
    t_kv("ferror.after_fgetc_on_w", ferror(f) != 0);
    t_kv("feof.after_fgetc_on_w", feof(f));
    clearerr(f);
    t_kv("ferror.cleared", ferror(f));
    t_kv("fclose.w", fclose(f));

    f = fopen("c.txt", "r");
    int c, n = 0;
    unsigned sum = 0;
    while ((c = fgetc(f)) != EOF) {
        n++;
        sum = sum * 31u + (unsigned)c;
    }
    t_kv("fgetc.n", n);
    t_kv("fgetc.sum", (long)sum);
    t_kv("feof.r", feof(f) != 0);
    t_kv("ferror.r", ferror(f));
    t_kv("fgetc.sticky_eof", fgetc(f));
    clearerr(f);
    t_kv("feof.cleared", feof(f));
    t_kv("fgetc.after_clearerr", fgetc(f));
    errno = 0;
    t_kv("fputc.on_r", fputc('z', f));
    t_kv("fputc.on_r_is_EBADF", errno == EBADF);
    t_kv("ferror.after_fputc_on_r", ferror(f) != 0);
    fclose(f);

    f = fopen("c.txt", "r");
    char b[8];
    memset(b, 'k', sizeof b);
    for (int i = 0; i < 12; i++) {
        char *r = fgets(b, sizeof b, f);
        if (!r) {
            line("fgets.null", b);
            break;
        }
        line("fgets", b);
    }
    t_kv("fgets.feof", feof(f) != 0);
    fclose(f);

    f = fopen("c.txt", "r");
    memset(b, 'k', sizeof b);
    t_kv("fgets.n1", fgets(b, 1, f) == b);
    t_kv("fgets.n1.b0", b[0]);
    t_kv("fgets.n1.b1", b[1]);
    t_kv("fgets.n2", fgets(b, 2, f) == b);
    line("fgets.n2", b);
    t_kv("fgets.n0_null", fgets(b, 0, f) == NULL);
    t_kv("getc.next", getc(f));
    fclose(f);

    f = fopen("empty.txt", "w");
    fclose(f);
    f = fopen("empty.txt", "r");
    memcpy(b, "kept", 5);
    t_kv("fgets.empty_null", fgets(b, sizeof b, f) == NULL);
    line("fgets.empty_untouched", b);
    t_kv("fgets.empty_feof", feof(f) != 0);
    fclose(f);

    /* stdin, from stdio_char.in */
    t_kv("getchar.1", getchar());
    t_kv("getc.stdin", getc(stdin));
    memset(b, 0, sizeof b);
    fgets(b, sizeof b, stdin);
    line("fgets.stdin", b);
    n = 0;
    while (getchar() != EOF)
        n++;
    t_kv("stdin.rest", n);
    t_kv("stdin.feof", feof(stdin) != 0);
    t_kv("stdin.sticky", getchar());
    errno = 0;
    t_kv("stdin.putc", fputc('x', stdin));
    t_kv("stdin.putc_is_EBADF", errno == EBADF);
    return 0;
}
