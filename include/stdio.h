/* muzzle: <stdio.h> (C17 7.21, POSIX.1-2024 XBD stdio.h for fdopen and
 * fileno), the functions muzzle implements. Hand-written from the
 * standards' synopses (ruling R3).
 *
 * FILE is opaque: C holds only pointers to it. The three standard
 * streams are objects muzzle reserves; stdin, stdout and stderr are
 * pointers to them (C17 7.21.1p3 asks for expressions of type FILE *).
 *
 * Not provided yet: the floating conversions of the printf family
 * (f F e E g G a A, and the L length), which lc03 owns; a call that
 * meets one returns a negative value with errno EINVAL and writes none
 * of its output (docs/LEDGER.md). Nor positional arguments (%n$), the
 * scanf family, positioning (fseek, ftell, fgetpos, fsetpos, rewind),
 * freopen, remove, rename, tmpfile, tmpnam, perror, getline. */
#ifndef _MUZZLE_STDIO_H
#define _MUZZLE_STDIO_H

#ifndef _MUZZLE_SIZE_T
#define _MUZZLE_SIZE_T
typedef __SIZE_TYPE__ size_t;
#endif
#ifndef _MUZZLE_VA_LIST
#define _MUZZLE_VA_LIST
typedef __builtin_va_list va_list;
#endif
#ifndef _MUZZLE_OFF_T
#define _MUZZLE_OFF_T
typedef long off_t;
#endif
#ifndef _MUZZLE_SSIZE_T
#define _MUZZLE_SSIZE_T
typedef long ssize_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct __muzzle_file FILE;
typedef struct { long __pos; } fpos_t;

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2
#define BUFSIZ 8192
#define EOF (-1)
#define FOPEN_MAX 16
#define FILENAME_MAX 4096
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
#define stdin stdin
#define stdout stdout
#define stderr stderr

/* C17 7.21.5 file access, and POSIX fdopen and fileno */
FILE *fopen(const char *restrict filename, const char *restrict mode);
FILE *fdopen(int fildes, const char *mode);
int fclose(FILE *stream);
int fflush(FILE *stream);
void setbuf(FILE *restrict stream, char *restrict buf);
int setvbuf(FILE *restrict stream, char *restrict buf, int mode, size_t size);
int fileno(FILE *stream);

/* C17 7.21.6 formatted output */
int printf(const char *restrict format, ...);
int fprintf(FILE *restrict stream, const char *restrict format, ...);
int sprintf(char *restrict s, const char *restrict format, ...);
int snprintf(char *restrict s, size_t n, const char *restrict format, ...);
int vprintf(const char *restrict format, va_list arg);
int vfprintf(FILE *restrict stream, const char *restrict format, va_list arg);
int vsprintf(char *restrict s, const char *restrict format, va_list arg);
int vsnprintf(char *restrict s, size_t n, const char *restrict format, va_list arg);

/* C17 7.21.7 character input/output */
int fgetc(FILE *stream);
char *fgets(char *restrict s, int n, FILE *restrict stream);
int fputc(int c, FILE *stream);
int fputs(const char *restrict s, FILE *restrict stream);
int getc(FILE *stream);
int getchar(void);
int putc(int c, FILE *stream);
int putchar(int c);
int puts(const char *s);
int ungetc(int c, FILE *stream);

/* C17 7.21.8 direct input/output */
size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);
size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);

/* C17 7.21.10 error-handling */
void clearerr(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);

#endif
