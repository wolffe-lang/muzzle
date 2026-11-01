#include <stdarg.h>
#include "t.h"
long q3_offsets(int, ...); unsigned long q3_bits(int, ...); long q3_ints(int, ...);
long q3_v(int, va_list);
static unsigned long bits(double d) { unsigned long u; memcpy(&u, &d, 8); return u; }
static long via(int n, ...) { va_list ap; va_start(ap, n); long r = q3_v(n, ap); int first = va_arg(ap, int); va_end(ap); return r * 10 + (first == 4); }
int main(void) {
    t_kv("q3.offsets", q3_offsets(1));
    unsigned long w2 = bits(1.5) ^ bits(2.0) ^ 7UL;
    unsigned long g2 = q3_bits(2, 1.5, 2.0, 7L);
    t_kv("q3.bits2_ok", g2 == w2);
    unsigned long w10 = 5UL; double d[10];
    for (int i = 0; i < 10; i++) { d[i] = 0.25 * (i + 1) + i * 1e10; w10 ^= bits(d[i]); }
    unsigned long g10 = q3_bits(10, d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], 5L);
    t_kv("q3.bits10_ok", g10 == w10);
    int x = 0;
    long r = q3_ints(0, -5L, 123456UL, "A", &x);
    t_kv("q3.ints", r); t_kv("q3.ints_store", x);
    t_kv("q3.v", via(3, 4, 5, 6));
    t_kv("q3.ok", q3_offsets(1) == 8048 && g2 == w2 && g10 == w10 && r == 516 && x == 77 && via(3, 4, 5, 6) == 151);
    return 0;
}
