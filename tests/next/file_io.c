/* The POSIX descriptor layer: XSH open, openat, read, write, close,
 * lseek, dup, dup2, pipe, isatty and access, with errno as each page
 * says. Descriptor numbers are printed: the lowest free one is POSIX's
 * rule (XSH 2.14), and both columns start with 0, 1 and 2 open. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHOW(what, expr) do { errno = 0; long r_ = (long)(expr); printf("%s r=%ld errno=%d\n", what, r_, errno); } while (0)

int main(void) {
    char buf[64];
    int fd = open("a.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    printf("open create fd=%d\n", fd);
    SHOW("write", write(fd, "hello, world\n", 13));
    SHOW("write zero", write(fd, "", 0));
    SHOW("lseek cur", lseek(fd, 0, SEEK_CUR));
    SHOW("read wronly", read(fd, buf, 4));
    SHOW("close", close(fd));
    SHOW("close again", close(fd));
    SHOW("close -1", close(-1));

    fd = open("a.txt", O_RDONLY);
    printf("open rdonly fd=%d\n", fd);
    memset(buf, 0, sizeof buf);
    SHOW("read 5", read(fd, buf, 5));
    printf("  [%s]\n", buf);
    SHOW("lseek set 7", lseek(fd, 7, SEEK_SET));
    memset(buf, 0, sizeof buf);
    SHOW("read rest", read(fd, buf, sizeof buf - 1));
    printf("  [%s]\n", buf);
    SHOW("read eof", read(fd, buf, 10));
    SHOW("lseek end -6", lseek(fd, -6, SEEK_END));
    SHOW("lseek cur +2", lseek(fd, 2, SEEK_CUR));
    SHOW("lseek negative", lseek(fd, -100, SEEK_SET));
    SHOW("lseek whence 9", lseek(fd, 0, 9));
    SHOW("lseek past end", lseek(fd, 100, SEEK_SET));
    SHOW("write rdonly", write(fd, "x", 1));

    /* dup shares the offset; dup2 picks the number. */
    int d = dup(fd);
    printf("dup fd=%d\n", d);
    SHOW("lseek via dup", lseek(d, 3, SEEK_SET));
    SHOW("offset seen by original", lseek(fd, 0, SEEK_CUR));
    SHOW("dup2 same", dup2(fd, fd));
    SHOW("dup2 to 50", dup2(fd, 50));
    SHOW("offset via 50", lseek(50, 0, SEEK_CUR));
    SHOW("dup2 onto open", dup2(d, 50));
    SHOW("dup2 bad old", dup2(77, 5));
    SHOW("dup2 bad new", dup2(fd, -1));
    SHOW("dup bad", dup(99));
    SHOW("close 50", close(50));
    SHOW("close dup", close(d));
    SHOW("close fd", close(fd));

    /* Lowest free number. */
    int a = open("a.txt", O_RDONLY), b = open("a.txt", O_RDONLY);
    close(a);
    int c = open("a.txt", O_RDONLY);
    printf("lowest a=%d b=%d c=%d\n", a, b, c);
    close(b);
    close(c);

    /* O_APPEND and O_EXCL, and the open errors. */
    fd = open("a.txt", O_WRONLY | O_APPEND);
    lseek(fd, 0, SEEK_SET);
    SHOW("append write", write(fd, "more\n", 5));
    SHOW("append offset", lseek(fd, 0, SEEK_CUR));
    close(fd);
    SHOW("open excl existing", open("a.txt", O_CREAT | O_EXCL | O_WRONLY, 0644));
    SHOW("open missing", open("missing.txt", O_RDONLY));
    SHOW("open empty name", open("", O_RDONLY));
    SHOW("open under file", open("a.txt/x", O_RDONLY));
    SHOW("mkdir d", mkdir("d", 0755));
    SHOW("open dir wronly", open("d", O_WRONLY));
    SHOW("open dir rdonly", (fd = open("d", O_RDONLY)));
    close(fd);
    SHOW("open file as dir", open("a.txt", O_RDONLY | O_DIRECTORY));
    fd = open("x.txt", O_CREAT | O_RDWR | O_CLOEXEC, 0600);
    SHOW("cloexec write", write(fd, "abcdef", 6));
    SHOW("pread-like lseek", lseek(fd, 2, SEEK_SET));
    SHOW("read 2", read(fd, buf, 2));
    printf("  [%.2s]\n", buf);
    close(fd);
    fd = open("x.txt", O_WRONLY | O_TRUNC);
    SHOW("truncated size", lseek(fd, 0, SEEK_END));
    close(fd);

    /* openat: relative to a held directory, and to AT_FDCWD. */
    int dfd = open("d", O_RDONLY | O_DIRECTORY);
    printf("dir fd=%d\n", dfd);
    fd = openat(dfd, "inner.txt", O_CREAT | O_WRONLY, 0600);
    printf("openat fd=%d\n", fd);
    SHOW("write inner", write(fd, "inside\n", 7));
    close(fd);
    fd = open("d/inner.txt", O_RDONLY);
    memset(buf, 0, sizeof buf);
    SHOW("read inner by path", read(fd, buf, sizeof buf - 1));
    printf("  [%s]\n", buf);
    close(fd);
    SHOW("openat cwd", (fd = openat(AT_FDCWD, "a.txt", O_RDONLY)));
    close(fd);
    SHOW("openat absolute ignores dir", (fd = openat(dfd, "/dev/null", O_RDONLY)));
    close(fd);
    SHOW("openat bad dirfd", openat(99, "a.txt", O_RDONLY));
    SHOW("openat file as dirfd", openat(0, "a.txt", O_RDONLY));
    close(dfd);

    /* pipe: bytes in order, end of file when the writer is gone. */
    int p[2] = {-1, -1};
    SHOW("pipe", pipe(p));
    printf("pipe fds=%d,%d\n", p[0], p[1]);
    SHOW("pipe write", write(p[1], "through", 7));
    memset(buf, 0, sizeof buf);
    SHOW("pipe read", read(p[0], buf, 3));
    printf("  [%s]\n", buf);
    SHOW("pipe lseek", lseek(p[0], 0, SEEK_SET));
    SHOW("isatty pipe", isatty(p[0]));
    close(p[1]);
    memset(buf, 0, sizeof buf);
    SHOW("pipe read rest", read(p[0], buf, sizeof buf - 1));
    printf("  [%s]\n", buf);
    SHOW("pipe eof", read(p[0], buf, 1));
    close(p[0]);

    /* isatty and access. */
    fd = open("a.txt", O_RDONLY);
    SHOW("isatty file", isatty(fd));
    close(fd);
    SHOW("isatty closed", isatty(fd));
    SHOW("access F_OK", access("a.txt", F_OK));
    SHOW("access R_OK", access("a.txt", R_OK));
    SHOW("access R_OK|W_OK", access("a.txt", R_OK | W_OK));
    SHOW("access X_OK file", access("a.txt", X_OK));
    SHOW("access X_OK dir", access("d", X_OK));
    SHOW("access missing", access("nope", F_OK));
    SHOW("access under file", access("a.txt/x", F_OK));
    SHOW("access bad mode", access("a.txt", 64));
    printf("F_OK=%d R_OK=%d W_OK=%d X_OK=%d\n", F_OK, R_OK, W_OK, X_OK);
    return 0;
}
