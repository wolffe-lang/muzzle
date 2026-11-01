/* muzzle: <wchar.h> (C17 7.29), the types only: printf's %lc and %ls
 * take a wint_t and a wchar_t pointer. No wide-character function is
 * provided yet (a locale lane's). Hand-written (ruling R3). */
#ifndef _MUZZLE_WCHAR_H
#define _MUZZLE_WCHAR_H

#include <stddef.h>

typedef __WINT_TYPE__ wint_t;
#define WEOF ((wint_t)-1)

#endif
