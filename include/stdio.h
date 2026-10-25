/* muzzle: C17 7.21.6.3 synopsis, restricted first printf slice.
 * Only bare %d %s %c %x %% are implemented; no streams yet. */
#ifndef _MUZZLE_STDIO_H
#define _MUZZLE_STDIO_H

int printf(const char * restrict format, ...);

#endif
