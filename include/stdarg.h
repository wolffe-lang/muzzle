/* muzzle: <stdarg.h> (C17 7.16). Hand-written from the standard's
 * synopsis (ruling R3). A variable argument list is the compiler's own
 * object, so the header names the compiler's builtins and nothing else;
 * on x86-64 the type is the psABI's (3.5.7), the record wolf calls
 * VaList ([abi.c.variadic.list]). */
#ifndef _MUZZLE_STDARG_H
#define _MUZZLE_STDARG_H

#ifndef _MUZZLE_VA_LIST
#define _MUZZLE_VA_LIST
typedef __builtin_va_list va_list;
#endif

#define va_start(ap, parmN) __builtin_va_start(ap, parmN)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_copy(dest, src) __builtin_va_copy(dest, src)
#define va_end(ap) __builtin_va_end(ap)

#endif
