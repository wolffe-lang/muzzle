/* muzzle: <fcntl.h> (POSIX.1-2024 XBD fcntl.h), open and its flags.
 * Hand-written from the standard's synopsis (ruling R3). The values are
 * Linux's, from the uapi headers asm-generic/fcntl.h and linux/fcntl.h
 * (numbers only). */
#ifndef _MUZZLE_FCNTL_H
#define _MUZZLE_FCNTL_H

#include <sys/types.h>

#define O_ACCMODE 03
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 0100
#define O_EXCL 0200
#define O_NOCTTY 0400
#define O_TRUNC 01000
#define O_APPEND 02000
#define O_NONBLOCK 04000
#define O_DSYNC 010000
#define O_DIRECTORY 0200000
#define O_NOFOLLOW 0400000
#define O_CLOEXEC 02000000
#define O_SYNC 04010000
#define O_TMPFILE 020200000

#define AT_FDCWD (-100)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* The mode argument is read only when O_CREAT or O_TMPFILE is given. */
int open(const char *path, int oflag, ...);

#endif
