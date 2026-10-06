/* muzzle: <unistd.h> (POSIX.1-2024 XBD unistd.h), the functions lc01
 * implements. Hand-written from the standard's synopsis (ruling R3). */
#ifndef _MUZZLE_UNISTD_H
#define _MUZZLE_UNISTD_H

#include <stddef.h>

typedef long ssize_t;

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

ssize_t write(int fildes, const void *buf, size_t nbyte);
_Noreturn void _exit(int status);

#endif
