#include <stdio.h>
int q_gt(unsigned long, unsigned long);
unsigned long q_div(unsigned long, unsigned long);
unsigned long q_add_small(unsigned long);
long q_as_i64(unsigned long);
int main(void) {
    unsigned long big = (unsigned long)-1 - 8, mid = 1UL << 63, cap = 1UL << 46;
    printf("gt(big,cap)=%d want 1\n", q_gt(big, cap));
    printf("gt(cap,big)=%d want 0\n", q_gt(cap, big));
    printf("gt(mid,cap)=%d want 1\n", q_gt(mid, cap));
    printf("div(big,3)=%lu want %lu\n", q_div(big, 3), big / 3);
    printf("add(mid)=%lu want %lu\n", q_add_small(mid), mid + 1);
    printf("as_i64(big)=%ld want %ld\n", q_as_i64(big), (long)big);
    return 0;
}
