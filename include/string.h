/* muzzle: <string.h> (C17 7.24), the functions muzzle implements.
 * Hand-written from the standard's synopsis. */
#ifndef _MUZZLE_STRING_H
#define _MUZZLE_STRING_H

#include <stddef.h>

void *memcpy(void *restrict s1, const void *restrict s2, size_t n);
void *memmove(void *s1, const void *s2, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int c);
char *strcpy(char *restrict s1, const char *restrict s2);

/* C17 7.24.6.2: glibc's text in the C locale; "Unknown error N" for a
 * number with none, in storage the next such call overwrites. */
char *strerror(int errnum);

#endif
