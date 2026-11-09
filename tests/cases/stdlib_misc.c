/* <stdlib.h> integer arithmetic and pseudo-random numbers: C17 7.22.6.1
 * abs, labs, llabs; 7.22.6.2 div, ldiv, lldiv (the quotient truncated
 * toward zero, quot * denom + rem == numer); 7.22.2 rand and srand.
 * C17 fixes no sequence for rand (7.22.2.1), so its values are never
 * printed: the properties C does fix are (the range, srand(1) restarting
 * the default sequence, a seed repeating its sequence). RAND_MAX is
 * glibc's, 2147483647, in both. */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    static const int iv[] = {0, 1, -1, 7, -7, INT_MAX, INT_MIN + 1, 100, -100};
    for (size_t i = 0; i < sizeof iv / sizeof *iv; i++)
        printf("abs(%d)=%d labs=%ld llabs=%lld\n", iv[i], abs(iv[i]), labs((long)iv[i] * 3), llabs((long long)iv[i] * 5));
    printf("labs(LONG_MIN+1)=%ld llabs(LLONG_MIN+1)=%lld\n", labs(LONG_MIN + 1), llabs(LLONG_MIN + 1));

    static const int num[] = {7, -7, 7, -7, 0, 1, INT_MAX, INT_MIN, INT_MIN, 100, -1};
    static const int den[] = {2, 2, -2, -2, 5, 1, 3, 3, 1, -7, INT_MAX};
    for (size_t i = 0; i < sizeof num / sizeof *num; i++) {
        div_t d = div(num[i], den[i]);
        ldiv_t l = ldiv((long)num[i] * 1000000007L, (long)den[i]);
        lldiv_t q = lldiv((long long)num[i] * 1048576, (long long)den[i] * 3);
        printf("div(%d,%d)=%d,%d ldiv=%ld,%ld lldiv=%lld,%lld\n", num[i], den[i], d.quot, d.rem, l.quot, l.rem, q.quot, q.rem);
    }
    printf("ldiv(LONG_MIN,7)=%ld,%ld\n", ldiv(LONG_MIN, 7).quot, ldiv(LONG_MIN, 7).rem);

    printf("RAND_MAX=%d\n", RAND_MAX);
    /* The default sequence is the one srand(1) starts (C17 7.22.2.2p2). */
    int first[100];
    for (int i = 0; i < 100; i++)
        first[i] = rand();
    srand(1);
    int same = 1, inrange = 1;
    for (int i = 0; i < 100; i++) {
        int r = rand();
        same &= r == first[i];
    }
    /* A seed repeats its sequence; values stay in [0, RAND_MAX]. */
    static int seq[1000];
    static const unsigned seeds[] = {0, 2, 12345, 4294967295u};
    int repeat = 1, differs = 0;
    for (size_t s = 0; s < sizeof seeds / sizeof *seeds; s++) {
        srand(seeds[s]);
        for (int i = 0; i < 1000; i++) {
            seq[i] = rand();
            inrange &= seq[i] >= 0 && seq[i] <= RAND_MAX;
        }
        srand(seeds[s]);
        for (int i = 0; i < 1000; i++)
            repeat &= rand() == seq[i];
        srand(seeds[s] + 1);
        for (int i = 0; i < 1000; i++)
            differs += rand() != seq[i];
    }
    /* Every bit position of the result takes both values. */
    int ones = 0, zeros = 0;
    srand(7);
    for (int i = 0; i < 20000; i++) {
        int r = rand();
        ones |= r;
        zeros |= ~r & RAND_MAX;
    }
    printf("rand default=srand1:%d inrange=%d repeat=%d differs=%d allbits=%d\n", same, inrange, repeat, differs > 3900,
           ones == RAND_MAX && zeros == RAND_MAX);
    return 0;
}
