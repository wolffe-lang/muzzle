/* muzzle: <unistd.h> (POSIX.1-2024 XBD unistd.h), the functions muzzle
 * implements. Hand-written from the standard's synopsis (ruling R3). */
#ifndef _MUZZLE_UNISTD_H
#define _MUZZLE_UNISTD_H

#include <stddef.h>
#include <sys/types.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

ssize_t read(int fildes, void *buf, size_t nbyte);
ssize_t write(int fildes, const void *buf, size_t nbyte);
off_t lseek(int fildes, off_t offset, int whence);
int close(int fildes);
int isatty(int fildes);
_Noreturn void _exit(int status);

#endif
