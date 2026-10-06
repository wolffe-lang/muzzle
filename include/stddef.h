/* muzzle: <stddef.h> (C17 7.19). Hand-written from the standard's
 * synopsis (ruling R3: the headers are muzzle's one permanent C).
 * The types come from the compiler's predefined macros. */
#ifndef _MUZZLE_STDDEF_H
#define _MUZZLE_STDDEF_H

typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __WCHAR_TYPE__ wchar_t;
typedef struct { long long __ll; long double __ld; } max_align_t;

#define NULL ((void *)0)
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif
