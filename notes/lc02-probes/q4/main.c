#include "t.h"
unsigned long q4_mag(long); unsigned long q4_bits(unsigned long); unsigned long q4_wrap(unsigned long); long q4_narrow(int);
int main(void) {
    unsigned long m = q4_mag(-9223372036854775807L - 1);
    t_kv("q4.mag_min_ok", m == 9223372036854775808UL);
    t_kv("q4.mag", (long)q4_mag(-42));
    unsigned long x = 0x123456789abcdef0UL;
    t_kv("q4.bits_ok", q4_bits(x) == (((x >> 4) & 15) | ((x << 8) ^ 1) | (~x & 255)));
    t_kv("q4.wrap_ok", q4_wrap(1) == ~0UL);
    t_kv("q4.narrow", q4_narrow(0x1ff80));
    t_kv("q4.ok", m == 9223372036854775808UL && q4_mag(-42) == 42 && q4_wrap(1) == ~0UL && q4_narrow(0x1ff80) == -128L * 100000 + 0xff80);
    return 0;
}
