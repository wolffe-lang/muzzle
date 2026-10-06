/* The cases' output helper: no printf in lc01 (no varargs yet), so a
 * case writes its lines through write(2). Built into both columns, so
 * the same code prints for glibc and for muzzle. Only deterministic
 * facts are printed: no addresses, comparisons as their sign. */
#ifndef MUZZLE_T_H
#define MUZZLE_T_H

#include <stddef.h>
#include <string.h>
#include <unistd.h>

static void t_str(const char *s) { write(1, s, strlen(s)); }

static void t_int(long v) {
    char b[24];
    int i = 23;
    unsigned long u = v < 0 ? 0UL - (unsigned long)v : (unsigned long)v;
    b[i] = 0;
    do {
        b[--i] = (char)('0' + u % 10);
        u /= 10;
    } while (u != 0);
    if (v < 0)
        b[--i] = '-';
    t_str(b + i);
}

static void t_kv(const char *k, long v) {
    t_str(k);
    t_str("=");
    t_int(v);
    t_str("\n");
}

static int t_sign(int v) { return (v > 0) - (v < 0); }

/* A bytewise digest of a buffer, so a long result is one line. */
static unsigned long t_digest(const void *p, size_t n) {
    const unsigned char *s = p;
    unsigned long h = 14695981039346656037UL;
    for (size_t i = 0; i < n; i++) {
        h ^= s[i];
        h *= 1099511628211UL;
    }
    return h >> 1;
}

#endif
