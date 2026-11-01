/* snprintf and sprintf, C17 7.21.6.5 and 7.21.6.6: at most n-1 bytes
 * and a null; n of 0 writes nothing (s may be null); the value is the
 * length that would have been written. Canary bytes on both sides show
 * nothing is written outside the bound; every bound from 0 to past the
 * length is tried for each format. An embedded null from %c counts. */
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wformat-zero-length"
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "t.h"

static unsigned char area[80];

/* The 80-byte area as hex pairs, '.' for an untouched canary byte. */
static void dump(const char *k, int r, int n) {
    static const char hex[] = "0123456789abcdef";
    char line[200];
    int at = 0;
    for (int i = 0; i < 40; i++) {
        unsigned char c = area[i];
        if (c == 0xA5) {
            line[at++] = '.';
            line[at++] = '.';
        } else {
            line[at++] = hex[c >> 4];
            line[at++] = hex[c & 15];
        }
    }
    line[at] = 0;
    t_str(k);
    t_kv(".r", r);
    t_kv("  n", n);
    t_str("  ");
    t_str(line);
    t_str("\n");
}

static void bounds(const char *k, const char *fmt, int a, const char *s) {
    for (int n = 0; n <= 18; n++) {
        memset(area, 0xA5, sizeof area);
        int r = snprintf((char *)area + 8, (size_t)n, fmt, a, s);
        dump(k, r, n);
    }
}

int main(void) {
    bounds("d-s", "%d-%s", -42, "wolf");
    bounds("pad", "[%8d|%-6s]", 7, "ab");
    bounds("nul", "a%cb%s", 0, "c");
    bounds("empty", "%.0d%s", 0, "");

    memset(area, 0xA5, sizeof area);
    int r = snprintf(NULL, 0, "%d %s", 123456, "abc");
    t_kv("null0.r", r);
    r = snprintf((char *)area + 8, 0, "%s", "zzz");
    dump("zero_no_write", r, 0);
    r = sprintf((char *)area + 8, "%s=%x", "key", 0xbeef);
    dump("sprintf", r, -1);
    r = sprintf((char *)area + 8, "");
    dump("sprintf.empty", r, -1);
    r = sprintf((char *)area + 8, "%c%c%c", 'a', 0, 'b');
    dump("sprintf.nul", r, -1);

    char big[600];
    memset(big, 0, sizeof big);
    r = snprintf(big, sizeof big, "%500d|%s", 1, "end");
    t_kv("wide.r", r);
    t_kv("wide.len", (long)strlen(big));
    t_str(big + 495);
    t_str("\n");
    r = snprintf(big, 10, "%500d|%s", 1, "end");
    t_kv("wide.trunc.r", r);
    t_kv("wide.trunc.len", (long)strlen(big));
    memset(area, 0xA5, sizeof area);
    errno = 0;
    r = snprintf((char *)area + 8, 6, "ab%lc", (wint_t)0x41);
    dump("lc.ok", r, 6);
    errno = 0;
    r = snprintf((char *)area + 8, 6, "%d%n", 12345, &r);
    dump("n", r, 6);
    errno = 0;
    memset(area, 0xA5, sizeof area);
    r = snprintf((char *)area + 8, (size_t)INT_MAX + 10, "%s", "huge n");
    dump("huge_n", r, -2);
    return 0;
}
