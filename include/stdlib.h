/* muzzle: <stdlib.h> (C17 7.22), the functions lc01 implements.
 * Hand-written from the standard's synopsis (ruling R3). */
#ifndef _MUZZLE_STDLIB_H
#define _MUZZLE_STDLIB_H

#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

/* exit flushes every open stream, then ends the process (no atexit
 * handlers yet: C function pointers, wolf-lang#520). _Exit does not. */
_Noreturn void exit(int status);
_Noreturn void _Exit(int status);

#endif
