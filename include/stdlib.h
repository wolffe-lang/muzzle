/* muzzle: <stdlib.h> (C17 7.22, POSIX.1-2024 XBD stdlib.h), the functions
 * muzzle implements. Hand-written from the standards' synopses: the
 * headers are muzzle's one permanent C.
 *
 * strtod is correctly rounded; RAND_MAX is glibc's; qsort keeps equal
 * elements in their original order (docs/LEDGER.md). */
#ifndef _MUZZLE_STDLIB_H
#define _MUZZLE_STDLIB_H

#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 2147483647

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

typedef struct {
    long long quot;
    long long rem;
} lldiv_t;

/* 7.22.1 numeric conversions */
double strtod(const char *restrict nptr, char **restrict endptr);
long strtol(const char *restrict nptr, char **restrict endptr, int base);
long long strtoll(const char *restrict nptr, char **restrict endptr, int base);
unsigned long strtoul(const char *restrict nptr, char **restrict endptr, int base);
unsigned long long strtoull(const char *restrict nptr, char **restrict endptr, int base);
int atoi(const char *nptr);
long atol(const char *nptr);
long long atoll(const char *nptr);

/* 7.22.2 pseudo-random sequences */
int rand(void);
void srand(unsigned int seed);

/* 7.22.3 memory management; POSIX posix_memalign */
void *aligned_alloc(size_t alignment, size_t size);
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
int posix_memalign(void **memptr, size_t alignment, size_t size);

/* 7.22.4 communication with the environment; POSIX setenv, unsetenv.
 * exit runs the atexit handlers, last first, then flushes every open
 * stream; _Exit and abort do neither. */
_Noreturn void abort(void);
int atexit(void (*func)(void));
_Noreturn void exit(int status);
_Noreturn void _Exit(int status);
char *getenv(const char *name);
int setenv(const char *envname, const char *envval, int overwrite);
int unsetenv(const char *name);

/* 7.22.5 searching and sorting */
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

/* 7.22.6 integer arithmetic */
int abs(int j);
long labs(long j);
long long llabs(long long j);
div_t div(int numer, int denom);
ldiv_t ldiv(long numer, long denom);
lldiv_t lldiv(long long numer, long long denom);

#endif
