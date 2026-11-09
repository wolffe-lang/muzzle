/* muzzle: <unistd.h> (POSIX.1-2024 XBD unistd.h), the functions muzzle
 * implements. Hand-written from the standard's synopsis (ruling R3). */
#ifndef _MUZZLE_UNISTD_H
#define _MUZZLE_UNISTD_H

#include <stddef.h>
#include <sys/types.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* access's modes: the bits it tests against "other"'s permission bits
 * (S_IROTH 4, S_IWOTH 2, S_IXOTH 1, uapi linux/stat.h; access(2)). */
#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK 1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

ssize_t read(int fildes, void *buf, size_t nbyte);
ssize_t write(int fildes, const void *buf, size_t nbyte);
off_t lseek(int fildes, off_t offset, int whence);
int close(int fildes);
int isatty(int fildes);
_Noreturn void _exit(int status);
int access(const char *path, int amode);
int chdir(const char *path);
char *getcwd(char *buf, size_t size);
int dup(int fildes);
int dup2(int fildes, int fildes2);
int pipe(int fildes[2]);
int rmdir(const char *path);
int unlink(const char *path);
int symlink(const char *path1, const char *path2);
ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize);

#endif
