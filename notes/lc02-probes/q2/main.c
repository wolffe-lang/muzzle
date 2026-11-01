#include "t.h"
struct F { int fd; int flags; unsigned char *buf; long n; };
long q2_size(void); int q2_get(struct F *); long q2_deref(struct F *);
void q2_set(struct F *, long); int q2_buf0(struct F *); void q2_put(struct F *, int);
int main(void) {
    unsigned char b[8] = {90, 0};
    struct F f = {5, 1, b, 0};
    long want = (long)sizeof(struct F) * 10000 + (long)offsetof(struct F, buf) * 100 + (long)offsetof(struct F, n);
    t_kv("q2.size", q2_size()); t_kv("q2.size_ok", q2_size() == want);
    t_kv("q2.get", q2_get(&f)); t_kv("q2.buf0", q2_buf0(&f));
    q2_set(&f, 2); t_kv("q2.after_set.n", f.n); t_kv("q2.after_set.flags", f.flags);
    t_kv("q2.deref", q2_deref(&f));
    q2_put(&f, 0x141); t_kv("q2.put.byte", b[2]); t_kv("q2.put.n", f.n);
    t_kv("q2.ok", f.n == 3 && f.flags == 5 && b[2] == 0x41 && q2_get(&f) == 5 && q2_buf0(&f) == 90);
    return 0;
}
