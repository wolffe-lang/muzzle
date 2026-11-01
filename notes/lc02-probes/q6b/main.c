#include "t.h"
typedef struct { unsigned gp, fp; void *ov, *rs; } rec;
unsigned long q6_mixed(rec *, int, ...);
static unsigned long bits(double d) { unsigned long u; memcpy(&u, &d, 8); return u; }
int main(void) {
    rec r; double d[10]; long v[10]; unsigned long want = 0;
    for (int i = 0; i < 10; i++) { d[i] = 1.0 / (i + 3) + i * 1e100; v[i] = (long)i * 1000003 - 7; want ^= bits(d[i]); want ^= (unsigned long)v[i]; }
    unsigned long got = q6_mixed(&r, 10, d[0], v[0], d[1], v[1], d[2], v[2], d[3], v[3], d[4], v[4],
                                 d[5], v[5], d[6], v[6], d[7], v[7], d[8], v[8], d[9], v[9]);
    t_kv("q6.ok", got == want);
    return 0;
}
