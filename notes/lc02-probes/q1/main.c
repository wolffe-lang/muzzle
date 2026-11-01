#include "t.h"
struct Pair { long a, b; };
long q1_twice(long); int q1_first(const unsigned char *); long q1_pair(struct Pair *);
int main(void) {
    struct Pair p = {3, 4};
    t_kv("q1.twice", q1_twice(21)); t_kv("q1.first", q1_first((const unsigned char *)"A"));
    t_kv("q1.pair", q1_pair(&p));
    t_kv("q1.ok", q1_twice(21) == 42 && q1_first((const unsigned char *)"A") == 65 && q1_pair(&p) == 7);
    return 0;
}
