/* muzzle: <sys/types.h> (POSIX.1-2024 XBD sys/types.h), the types the
 * headers lc01 and lc02 declare need. Hand-written from the synopsis
 * (ruling R3). Sizes are the x86-64 psABI's: off_t and ssize_t are long,
 * mode_t is unsigned int, pid_t is int. */
#ifndef _MUZZLE_SYS_TYPES_H
#define _MUZZLE_SYS_TYPES_H

#ifndef _MUZZLE_SIZE_T
#define _MUZZLE_SIZE_T
typedef __SIZE_TYPE__ size_t;
#endif
#ifndef _MUZZLE_SSIZE_T
#define _MUZZLE_SSIZE_T
typedef long ssize_t;
#endif
#ifndef _MUZZLE_OFF_T
#define _MUZZLE_OFF_T
typedef long off_t;
#endif
#ifndef _MUZZLE_MODE_T
#define _MUZZLE_MODE_T
typedef unsigned int mode_t;
#endif
#ifndef _MUZZLE_PID_T
#define _MUZZLE_PID_T
typedef int pid_t;
#endif

#endif
