#include "t.h"
unsigned long q7_shl(unsigned long, unsigned); unsigned long q7_shr(unsigned long); unsigned long q7_mulw(unsigned long);
int main(void) {
    unsigned long x = 0xf23456789abcdef1UL;
    t_kv("q7.shl_ok", q7_shl(x, 8) == x << 8); t_kv("q7.shr", (long)q7_shr(x)); t_kv("q7.mulw_ok", q7_mulw(x) == x * 10);
    return 0;
}
