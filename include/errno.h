/* muzzle: <errno.h> (C17 7.5, POSIX.1-2024 XBD errno.h).
 * Hand-written (ruling R3). The numbers are Linux's, from the uapi
 * header asm-generic/errno-base.h (linux-api-headers 7.1, numbers only).
 *
 * errno is one process-wide int in this first cut (ruling R4's
 * single-threaded errno); lc05 makes it per thread behind the same
 * __errno_location, so this header does not change. */
#ifndef _MUZZLE_ERRNO_H
#define _MUZZLE_ERRNO_H

int *__errno_location(void);
#define errno (*__errno_location())

#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define ENXIO 6
#define E2BIG 7
#define ENOEXEC 8
#define EBADF 9
#define ECHILD 10
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define ENOTBLK 15
#define EBUSY 16
#define EEXIST 17
#define EXDEV 18
#define ENODEV 19
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ENFILE 23
#define EMFILE 24
#define ENOTTY 25
#define ETXTBSY 26
#define EFBIG 27
#define ENOSPC 28
#define ESPIPE 29
#define EROFS 30
#define EMLINK 31
#define EPIPE 32
#define EDOM 33
#define ERANGE 34

#endif
