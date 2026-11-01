#include "t.h"
long q2b_fill(unsigned char *, long);
int main(void) {
    unsigned char b[4] = {0, 0, 0, 0};
    long n = q2b_fill(b, 2);
    t_kv("q2b.count", n); t_kv("q2b.b0", b[0]); t_kv("q2b.b1", b[1]); t_kv("q2b.b2", b[2]);
    t_kv("q2b.ok", n == 3 && b[0] == 'h' && b[1] == 'i' && b[2] == 0);
    return 0;
}
