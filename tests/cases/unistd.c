/* The descriptor calls under stdio, POSIX.1-2024 XSH open, close, read,
 * lseek, isatty: each answers the kernel's result, or -1 with errno set.
 * The file it writes (u.txt) is compared by the harness. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "t.h"

int main(void) {
    int fd = open("u.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    t_kv("open.creat.fd_is_3", fd == 3);
    t_kv("write.10", write(fd, "0123456789", 10));
    errno = 0;
    t_kv("read.on_wronly", read(fd, (void *)0, 0) < 0 ? -1 : 0);
    t_kv("read.on_wronly_is_EBADF", errno == EBADF);
    t_kv("close", close(fd));
    errno = 0;
    t_kv("close.again", close(fd));
    t_kv("close.again_is_EBADF", errno == EBADF);

    fd = open("u.txt", O_RDONLY);
    char b[16];
    memset(b, 0, sizeof b);
    t_kv("read.4", read(fd, b, 4));
    t_str(b); t_str("\n");
    t_kv("lseek.cur", lseek(fd, 0, SEEK_CUR));
    t_kv("lseek.set8", lseek(fd, 8, SEEK_SET));
    t_kv("read.rest", read(fd, b, sizeof b));
    t_kv("read.eof", read(fd, b, sizeof b));
    t_kv("lseek.end-3", lseek(fd, -3, SEEK_END));
    t_kv("lseek.past_end", lseek(fd, 100, SEEK_SET));
    t_kv("read.past_end", read(fd, b, 1));
    errno = 0;
    t_kv("lseek.negative", lseek(fd, -100, SEEK_SET));
    t_kv("lseek.negative_is_EINVAL", errno == EINVAL);
    errno = 0;
    t_kv("lseek.whence99", lseek(fd, 0, 99));
    t_kv("lseek.whence99_is_EINVAL", errno == EINVAL);
    errno = 0;
    t_kv("write.on_rdonly", write(fd, "x", 1));
    t_kv("write.on_rdonly_is_EBADF", errno == EBADF);
    errno = 0;
    t_kv("isatty.file", isatty(fd));
    t_kv("isatty.file_is_ENOTTY", errno == ENOTTY);
    errno = 0;
    t_kv("isatty.99", isatty(99));
    t_kv("isatty.99_is_EBADF", errno == EBADF);
    errno = 0;
    t_kv("isatty.stdout_file", isatty(1));
    t_kv("isatty.stdout_file_is_ENOTTY", errno == ENOTTY);
    errno = 0;
    lseek(fd, 0, SEEK_SET);
    t_kv("read.efault", read(fd, (void *)8, 4));
    t_kv("read.efault_is_EFAULT", errno == EFAULT);
    t_kv("close.rdonly", close(fd));

    errno = 0;
    t_kv("open.enoent", open("no/such/file", O_RDONLY));
    t_kv("open.enoent_is_ENOENT", errno == ENOENT);
    errno = 0;
    t_kv("open.excl", open("u.txt", O_WRONLY | O_CREAT | O_EXCL, 0644));
    t_kv("open.excl_is_EEXIST", errno == EEXIST);
    errno = 0;
    t_kv("open.dir_for_write", open(".", O_WRONLY));
    t_kv("open.dir_for_write_is_EISDIR", errno == EISDIR);

    fd = open("v.txt", O_RDWR | O_CREAT | O_APPEND, 0600);
    t_kv("append.fd_is_3", fd == 3);
    write(fd, "abc", 3);
    lseek(fd, 0, SEEK_SET);
    write(fd, "def", 3);
    t_kv("append.offset", lseek(fd, 0, SEEK_CUR));
    lseek(fd, 0, SEEK_SET);
    memset(b, 0, sizeof b);
    t_kv("append.read", read(fd, b, sizeof b));
    t_str(b); t_str("\n");
    close(fd);

    fd = open("v.txt", O_WRONLY | O_TRUNC);
    t_kv("trunc.size", lseek(fd, 0, SEEK_END));
    close(fd);
    errno = 0;
    t_kv("read.badfd", read(-1, b, 1));
    t_kv("read.badfd_is_EBADF", errno == EBADF);
    return 0;
}
