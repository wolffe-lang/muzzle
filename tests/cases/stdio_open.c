/* fopen's modes (C17 7.21.5.3, POSIX XSH fopen), fdopen (POSIX), fclose,
 * fileno: what each mode opens, creates, truncates and appends, and the
 * errors. Reports through write(2) (t.h), never through stdout's stream.
 * The files it leaves are compared by the harness. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "t.h"

static void put_file(const char *name, const char *text) {
    FILE *f = fopen(name, "w");
    fputs(text, f);
    fclose(f);
}

/* The first line of a file, or "-" when it does not open. */
static void show(const char *k, const char *name) {
    char b[64];
    FILE *f = fopen(name, "r");
    t_str(k);
    t_str("=");
    if (!f) {
        t_str("-\n");
        return;
    }
    if (fgets(b, sizeof b, f))
        t_str(b);
    else
        t_str("(empty)");
    t_str("\n");
    fclose(f);
}

int main(void) {
    FILE *f;
    put_file("a.txt", "alpha");
    show("w.creates", "a.txt");
    put_file("a.txt", "be");
    show("w.truncates", "a.txt");

    f = fopen("a.txt", "a");
    fputs("ta", f);
    t_kv("a.fclose", fclose(f));
    show("a.appends", "a.txt");

    f = fopen("a.txt", "r");
    t_kv("r.getc", fgetc(f));
    errno = 0;
    t_kv("r.putc", fputc('x', f));
    t_kv("r.putc_is_EBADF", errno == EBADF);
    fclose(f);

    f = fopen("a.txt", "r+");
    t_kv("r+.getc", fgetc(f));
    t_kv("r+.fflush", fflush(f));
    t_kv("r+.putc", fputc('E', f));
    fclose(f);
    show("r+.wrote_at_1", "a.txt");

    f = fopen("b.txt", "w+");
    fputs("wplus", f);
    fflush(f);
    lseek(fileno(f), 0, SEEK_SET);
    t_kv("w+.getc", fgetc(f));
    fclose(f);

    put_file("c.txt", "cc");
    f = fopen("c.txt", "a+");
    t_kv("a+.getc_from_start", fgetc(f));
    fflush(f);
    fputs("dd", f);
    fclose(f);
    show("a+.appends", "c.txt");

    const char *modes[] = {"rb", "wb", "ab", "r+b", "rb+", "w+b", "wb+", "a+b", "ab+", "rw", "r+bx", "rbe"};
    for (unsigned i = 0; i < sizeof modes / sizeof modes[0]; i++) {
        errno = 0;
        f = fopen("d.txt", modes[i]);
        t_str("mode.");
        t_str(modes[i]);
        t_kv("", f != NULL);
        if (f)
            fclose(f);
    }

    errno = 0;
    f = fopen("x.txt", "wx");
    t_kv("wx.new", f != NULL);
    if (f)
        fclose(f);
    errno = 0;
    f = fopen("x.txt", "wx");
    t_kv("wx.exists", f != NULL);
    t_kv("wx.exists_is_EEXIST", errno == EEXIST);

    const char *bad[] = {"", "z", "+", "b", "x"};
    for (unsigned i = 0; i < sizeof bad / sizeof bad[0]; i++) {
        errno = 0;
        f = fopen("a.txt", bad[i]);
        t_str("badmode.");
        t_str(bad[i]);
        t_kv("", f == NULL && errno == EINVAL);
    }
    errno = 0;
    f = fopen("no/such", "r");
    t_kv("enoent", f == NULL && errno == ENOENT);
    errno = 0;
    f = fopen("no/such", "w");
    t_kv("enoent.w", f == NULL && errno == ENOENT);
    errno = 0;
    f = fopen(".", "w");
    t_kv("eisdir", f == NULL && errno == EISDIR);

    int fd = open("a.txt", O_RDONLY);
    errno = 0;
    f = fdopen(fd, "w");
    t_kv("fdopen.w_on_rdonly", f != NULL);
    t_kv("fdopen.w_on_rdonly_is_EINVAL", errno == EINVAL);
    errno = 0;
    f = fdopen(fd, "q");
    t_kv("fdopen.badmode", f == NULL && errno == EINVAL);
    f = fdopen(fd, "r");
    t_kv("fdopen.r.fileno_matches", fileno(f) == fd);
    t_kv("fdopen.r.getc", fgetc(f));
    t_kv("fdopen.fclose", fclose(f));
    errno = 0;
    t_kv("fdopen.fclose_closed_fd", close(fd));
    t_kv("fdopen.fclose_closed_fd_is_EBADF", errno == EBADF);
    errno = 0;
    f = fdopen(99, "r");
    t_kv("fdopen.badfd", f == NULL && errno == EBADF);
    fd = open("e.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(fd, "12", 2);
    f = fdopen(fd, "a");
    fputs("34", f);
    fclose(f);
    show("fdopen.a", "e.txt");
    fd = open("e.txt", O_RDWR);
    f = fdopen(fd, "w");
    fputs("Z", f);
    fclose(f);
    show("fdopen.w_does_not_truncate", "e.txt");

    t_kv("fileno.std", fileno(stdin) * 100 + fileno(stdout) * 10 + fileno(stderr));
    for (int i = 0; i < 20; i++) {
        f = fopen("a.txt", "r");
        t_kv("many.fd", fileno(f));
        fclose(f);
    }
    FILE *many[20];
    for (int i = 0; i < 20; i++)
        many[i] = fopen("a.txt", "r");
    int all = 1;
    for (int i = 0; i < 20; i++)
        all = all && many[i] && fileno(many[i]) == 3 + i;
    t_kv("many.open_at_once", all);
    for (int i = 19; i >= 0; i--)
        fclose(many[i]);
    return 0;
}
