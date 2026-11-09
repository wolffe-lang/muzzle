/* The POSIX file-system calls: XSH stat, fstat, lstat, unlink, rename,
 * mkdir, rmdir, symlink and readlink, with errno as each page says.
 * Only stable fields are printed: the type, the permission bits (under
 * the umask both columns share), the size and the link count; the
 * times are checked against time() around the write, never printed.
 * Everything made is removed again, so the directories compare. */
#define _DEFAULT_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define SHOW(what, expr) do { errno = 0; long r_ = (long)(expr); printf("%s r=%ld errno=%d\n", what, r_, errno); } while (0)

static const char *kind(mode_t m) {
    return S_ISREG(m) ? "reg" : S_ISDIR(m) ? "dir" : S_ISLNK(m) ? "lnk" : S_ISCHR(m) ? "chr" : S_ISBLK(m) ? "blk"
         : S_ISFIFO(m) ? "fifo" : S_ISSOCK(m) ? "sock" : "?";
}

static void st(const char *what, int r, const struct stat *s) {
    if (r != 0) {
        printf("%s r=%d errno=%d\n", what, r, errno);
        return;
    }
    printf("%s r=0 %s perm=%o size=%lld nlink=%lu\n", what, kind(s->st_mode), (unsigned)(s->st_mode & 07777),
           (long long)s->st_size, (unsigned long)s->st_nlink);
}

int main(void) {
    struct stat s;
    time_t t0 = time(NULL);
    int fd = open("f.txt", O_CREAT | O_WRONLY | O_TRUNC, 0640);
    write(fd, "0123456789", 10);
    errno = 0;
    st("fstat open", fstat(fd, &s), &s);
    close(fd);
    time_t t1 = time(NULL);
    errno = 0;
    st("stat f.txt", stat("f.txt", &s), &s);
    printf("mtime in window=%d atim nsec=%d\n", s.st_mtime >= t0 - 1 && s.st_mtime <= t1 + 1,
           s.st_atim.tv_nsec >= 0 && s.st_atim.tv_nsec < 1000000000);
    printf("blksize>0=%d blocks>=0=%d\n", s.st_blksize > 0, s.st_blocks >= 0);
    errno = 0;
    st("stat missing", stat("missing", &s), &s);
    errno = 0;
    st("stat under file", stat("f.txt/x", &s), &s);
    errno = 0;
    st("stat empty", stat("", &s), &s);
    errno = 0;
    st("fstat bad", fstat(77, &s), &s);
    errno = 0;
    st("stat .", stat(".", &s), &s);
    int p[2];
    pipe(p);
    errno = 0;
    st("fstat pipe", fstat(p[0], &s), &s);
    close(p[0]);
    close(p[1]);
    fd = open("/dev/null", O_RDONLY);
    errno = 0;
    st("fstat /dev/null", fstat(fd, &s), &s);
    close(fd);

    /* Symbolic links: lstat sees the link, stat its target. */
    SHOW("symlink", symlink("f.txt", "ln"));
    errno = 0;
    st("lstat ln", lstat("ln", &s), &s);
    errno = 0;
    st("stat ln", stat("ln", &s), &s);
    char buf[64];
    memset(buf, '#', sizeof buf);
    errno = 0;
    long n = (long)readlink("ln", buf, sizeof buf);
    printf("readlink r=%ld errno=%d [%.*s] next=%c\n", n, errno, n > 0 ? (int)n : 0, buf, buf[n > 0 ? n : 0]);
    errno = 0;
    n = (long)readlink("ln", buf, 3);
    printf("readlink short r=%ld [%.3s]\n", n, buf);
    SHOW("readlink not a link", readlink("f.txt", buf, sizeof buf));
    SHOW("readlink missing", readlink("missing", buf, sizeof buf));
    SHOW("symlink exists", symlink("x", "ln"));
    SHOW("symlink dangling", symlink("nowhere", "dangle"));
    errno = 0;
    st("lstat dangle", lstat("dangle", &s), &s);
    errno = 0;
    st("stat dangle", stat("dangle", &s), &s);
    SHOW("unlink dangle", unlink("dangle"));
    SHOW("unlink ln", unlink("ln"));
    errno = 0;
    st("stat f.txt after unlink ln", stat("f.txt", &s), &s);

    /* mkdir and rmdir. */
    SHOW("mkdir d", mkdir("d", 0750));
    errno = 0;
    st("stat d", stat("d", &s), &s);
    SHOW("mkdir d again", mkdir("d", 0750));
    SHOW("mkdir no parent", mkdir("x/y", 0750));
    SHOW("mkdir under file", mkdir("f.txt/y", 0750));
    SHOW("mkdir d/e", mkdir("d/e", 0700));
    errno = 0;
    st("stat d (2 links more)", stat("d", &s), &s);
    SHOW("rmdir non-empty", rmdir("d"));
    SHOW("rmdir file", rmdir("f.txt"));
    SHOW("rmdir missing", rmdir("missing"));
    SHOW("rmdir dot", rmdir("d/e/."));
    SHOW("unlink dir", unlink("d/e"));
    SHOW("rmdir d/e", rmdir("d/e"));

    /* rename. */
    fd = open("g.txt", O_CREAT | O_WRONLY, 0600);
    write(fd, "gg", 2);
    close(fd);
    SHOW("rename f->h", rename("f.txt", "h.txt"));
    SHOW("stat f gone", stat("f.txt", &s));
    SHOW("rename onto existing", rename("g.txt", "h.txt"));
    errno = 0;
    st("stat h", stat("h.txt", &s), &s);
    SHOW("rename missing", rename("g.txt", "i.txt"));
    SHOW("rename same", rename("h.txt", "h.txt"));
    SHOW("rename into dir", rename("h.txt", "d/h.txt"));
    SHOW("mkdir k", mkdir("k", 0755));
    SHOW("rename dir onto non-empty", rename("k", "d"));
    SHOW("rename file onto dir", rename("d/h.txt", "k"));
    fd = open("m.txt", O_CREAT | O_WRONLY, 0600);
    close(fd);
    SHOW("rename dir onto file", rename("k", "m.txt"));
    SHOW("rename dir into itself", rename("d", "d/sub"));
    SHOW("rename dir onto empty dir", rename("k", "d2"));
    SHOW("rmdir d2", rmdir("d2"));

    /* unlink. */
    SHOW("unlink missing", unlink("missing"));
    SHOW("unlink m", unlink("m.txt"));
    SHOW("unlink under file", unlink("d/h.txt/x"));
    SHOW("unlink d/h", unlink("d/h.txt"));
    SHOW("rmdir d", rmdir("d"));
    errno = 0;
    printf("umask-free mode bits checked: S_IRWXU=%o S_IFMT=%o S_IFDIR=%o S_IFREG=%o S_IFLNK=%o\n", S_IRWXU, S_IFMT,
           S_IFDIR, S_IFREG, S_IFLNK);
    return 0;
}
