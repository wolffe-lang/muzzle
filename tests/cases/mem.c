/* <string.h> memory functions, C17 7.24.2.1 memcpy, 7.24.2.2 memmove,
 * 7.24.6.1 memset, 7.24.4.1 memcmp. */
#include <string.h>
#include "t.h"

static unsigned char src[256], dst[256], buf[256];

static void fill(unsigned char *p, size_t n, unsigned seed) {
    for (size_t i = 0; i < n; i++)
        p[i] = (unsigned char)(seed + i * 7);
}

int main(void) {
    /* memcpy: every length 0..64 at every offset 0..7, return value is s1. */
    fill(src, sizeof src, 3);
    int ret_ok = 1;
    unsigned long h = 0;
    for (size_t off = 0; off < 8; off++)
        for (size_t n = 0; n <= 64; n++) {
            memset(dst, 0xAA, sizeof dst);
            if (memcpy(dst + off, src + (7 - off), n) != dst + off)
                ret_ok = 0;
            h ^= t_digest(dst, sizeof dst) + n * 31 + off;
        }
    t_kv("memcpy.ret", ret_ok);
    t_kv("memcpy.digest", (long)(h >> 1));

    /* memmove: overlapping both ways, every length 0..40, shifts 1..9. */
    ret_ok = 1;
    for (int shift = 1; shift <= 9; shift++) {
        unsigned long fwd = 0, bwd = 0;
        for (size_t n = 0; n <= 40; n++) {
            fill(buf, sizeof buf, (unsigned)n);
            if (memmove(buf + 100 + shift, buf + 100, n) != buf + 100 + shift)
                ret_ok = 0;
            fwd ^= t_digest(buf, sizeof buf) + n;
            fill(buf, sizeof buf, (unsigned)n);
            if (memmove(buf + 100, buf + 100 + shift, n) != buf + 100)
                ret_ok = 0;
            bwd ^= t_digest(buf, sizeof buf) + n;
        }
        t_str("memmove.shift");
        t_int(shift);
        t_kv(".up", (long)(fwd >> 1));
        t_str("memmove.shift");
        t_int(shift);
        t_kv(".down", (long)(bwd >> 1));
    }
    /* memmove onto itself, and a non-overlapping move. */
    fill(buf, sizeof buf, 9);
    memmove(buf + 10, buf + 10, 50);
    t_kv("memmove.self", (long)(t_digest(buf, sizeof buf) >> 1));
    memmove(buf, buf + 128, 100);
    t_kv("memmove.apart", (long)(t_digest(buf, sizeof buf) >> 1));
    t_kv("memmove.ret", ret_ok);

    /* memset: c is converted to unsigned char; n = 0 writes nothing. */
    memset(buf, 7, sizeof buf);
    t_kv("memset.ret", memset(buf + 3, 0x1ff, 10) == buf + 3);
    memset(buf + 20, -2, 5);
    memset(buf + 40, 'x', (0)); /* n = 0, parenthesized for -Wmemset-transposed-args */
    t_kv("memset.b2", buf[2]);
    t_kv("memset.b3", buf[3]);
    t_kv("memset.b12", buf[12]);
    t_kv("memset.b13", buf[13]);
    t_kv("memset.b20", buf[20]);
    t_kv("memset.b40", buf[40]);
    t_kv("memset.digest", (long)(t_digest(buf, sizeof buf) >> 1));

    /* memcmp: bytes compare as unsigned char; only the sign is fixed. */
    t_kv("memcmp.eq", t_sign(memcmp("abcdef", "abcdef", 6)));
    t_kv("memcmp.zero", t_sign(memcmp("a", "b", 0)));
    t_kv("memcmp.lt", t_sign(memcmp("abc", "abd", 3)));
    t_kv("memcmp.gt", t_sign(memcmp("abd", "abc", 3)));
    t_kv("memcmp.high", t_sign(memcmp("\x80", "\x01", 1)));
    t_kv("memcmp.past_nul", t_sign(memcmp("a\0b", "a\0c", 3)));
    t_kv("memcmp.first_diff", t_sign(memcmp("az", "by", 2)));
    return 0;
}
