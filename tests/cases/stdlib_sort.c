/* <stdlib.h> searching and sorting: C17 7.22.5.1 bsearch, 7.22.5.2 qsort.
 * C17 leaves the order of elements that compare equal unspecified
 * (7.22.5.2p4); glibc's qsort keeps them in their original order, and so
 * does muzzle's (docs/LEDGER.md), so the `ties` lines compare it. Large
 * results print as a digest. The comparator call count is never printed:
 * it is the algorithm's, not the interface's. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long seed;
static unsigned next(void) {
    seed = seed * 6364136223846793005UL + 1442695040888963407UL;
    return (unsigned)(seed >> 33);
}

static unsigned long digest(const void *p, size_t n) {
    const unsigned char *s = p;
    unsigned long h = 14695981039346656037UL;
    for (size_t i = 0; i < n; i++) {
        h ^= s[i];
        h *= 1099511628211UL;
    }
    return h >> 1;
}

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static int cmp_int_desc(const void *a, const void *b) { return cmp_int(b, a); }

static int cmp_uchar(const void *a, const void *b) {
    return *(const unsigned char *)a - *(const unsigned char *)b;
}

struct three { unsigned char k, a, b; };
static int cmp_three(const void *a, const void *b) {
    const struct three *x = a, *y = b;
    return (x->k > y->k) - (x->k < y->k);
}

struct rec { long key; long seq; char pad[8]; };
static int cmp_rec(const void *a, const void *b) {
    const struct rec *x = a, *y = b;
    return (x->key > y->key) - (x->key < y->key);
}

struct big { int key; int seq; char body[92]; };
static int cmp_big(const void *a, const void *b) {
    const struct big *x = a, *y = b;
    return (x->key > y->key) - (x->key < y->key);
}

static int cmp_str(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

static int sorted_int(const int *v, size_t n) {
    for (size_t i = 1; i < n; i++)
        if (v[i - 1] > v[i])
            return 0;
    return 1;
}

static int v[100000];

static void ints(size_t n, unsigned mod) {
    seed = n * 7 + mod;
    for (size_t i = 0; i < n; i++)
        v[i] = mod ? (int)(next() % mod) - (int)(mod / 2) : (int)next();
    qsort(v, n, sizeof *v, cmp_int);
    printf("ints n=%zu mod=%u sorted=%d digest=%lu\n", n, mod, sorted_int(v, n), digest(v, n * sizeof *v));
}

int main(void) {
    static const size_t ns[] = {0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 100, 1000, 10000, 100000};
    for (size_t i = 0; i < sizeof ns / sizeof *ns; i++) {
        ints(ns[i], 0);
        ints(ns[i], 5);
    }
    /* Already sorted, reversed, all equal. */
    for (int i = 0; i < 1000; i++)
        v[i] = i;
    qsort(v, 1000, sizeof *v, cmp_int);
    printf("presorted digest=%lu\n", digest(v, 1000 * sizeof *v));
    qsort(v, 1000, sizeof *v, cmp_int_desc);
    printf("reversed first=%d last=%d digest=%lu\n", v[0], v[999], digest(v, 1000 * sizeof *v));
    for (int i = 0; i < 1000; i++)
        v[i] = 42;
    qsort(v, 1000, sizeof *v, cmp_int);
    printf("equal digest=%lu\n", digest(v, 1000 * sizeof *v));

    /* Element sizes 1, 3, 24 and 100: every byte of an element moves. */
    static unsigned char bytes[5000];
    seed = 99;
    for (size_t i = 0; i < sizeof bytes; i++)
        bytes[i] = (unsigned char)next();
    qsort(bytes, sizeof bytes, 1, cmp_uchar);
    printf("size1 digest=%lu\n", digest(bytes, sizeof bytes));

    static struct three t[3000];
    seed = 5;
    for (int i = 0; i < 3000; i++) {
        t[i].k = (unsigned char)(next() % 40);
        t[i].a = (unsigned char)i;
        t[i].b = (unsigned char)(i >> 8);
    }
    qsort(t, 3000, sizeof *t, cmp_three);
    printf("size3 ties digest=%lu\n", digest(t, sizeof t));

    static struct rec r[20000];
    static const unsigned mods[] = {1, 2, 3, 10, 1000};
    for (size_t m = 0; m < sizeof mods / sizeof *mods; m++) {
        static const size_t rn[] = {2, 5, 33, 20000};
        for (size_t k = 0; k < sizeof rn / sizeof *rn; k++) {
            seed = mods[m] * 31 + rn[k];
            for (size_t i = 0; i < rn[k]; i++) {
                r[i].key = (long)(next() % mods[m]);
                r[i].seq = (long)i;
                memset(r[i].pad, (int)(i & 0x7f), sizeof r[i].pad);
            }
            qsort(r, rn[k], sizeof *r, cmp_rec);
            int stable = 1;
            for (size_t i = 1; i < rn[k]; i++)
                if (r[i].key == r[i - 1].key && r[i].seq < r[i - 1].seq)
                    stable = 0;
            printf("ties size24 n=%zu mod=%u stable=%d digest=%lu\n", rn[k], mods[m], stable, digest(r, rn[k] * sizeof *r));
        }
    }

    static struct big g[2000];
    seed = 77;
    for (int i = 0; i < 2000; i++) {
        g[i].key = (int)(next() % 50);
        g[i].seq = i;
        memset(g[i].body, i & 0xff, sizeof g[i].body);
    }
    qsort(g, 2000, sizeof *g, cmp_big);
    printf("ties size100 digest=%lu\n", digest(g, sizeof g));

    /* Strings through a comparator on char *. */
    static const char *words[] = {"pear", "apple", "fig", "banana", "apple", "", "Zebra", "zebra", "fig", "a", "ab", "abc", "b"};
    const char *w[sizeof words / sizeof *words];
    memcpy(w, words, sizeof words);
    qsort(w, sizeof w / sizeof *w, sizeof *w, cmp_str);
    printf("words:");
    for (size_t i = 0; i < sizeof w / sizeof *w; i++)
        printf(" [%s]", w[i]);
    printf("\n");

    /* bsearch on distinct sorted keys: every present key is found at its
     * index, every absent one is a null pointer. */
    for (int i = 0; i < 1000; i++)
        v[i] = i * 3;
    int found = 0, at = 1, absent = 1;
    for (int k = -3; k < 3003; k++) {
        int *p = bsearch(&k, v, 1000, sizeof *v, cmp_int);
        if (k % 3 == 0 && k >= 0 && k < 3000) {
            found += p != NULL;
            at &= p != NULL && p - v == k / 3;
        } else {
            absent &= p == NULL;
        }
    }
    printf("bsearch found=%d at=%d absent=%d\n", found, at, absent);
    int key = 5;
    printf("bsearch empty=%d\n", bsearch(&key, v, 0, sizeof *v, cmp_int) == NULL);
    for (size_t n = 1; n <= 9; n++) {
        int hit = 1;
        for (size_t i = 0; i < n; i++) {
            int k = v[i];
            int *p = bsearch(&k, v, n, sizeof *v, cmp_int);
            hit &= p == &v[i];
        }
        int lo = -1, hi = 3 * (int)n;
        printf("bsearch n=%zu hit=%d below=%d above=%d\n", n, hit,
               bsearch(&lo, v, n, sizeof *v, cmp_int) == NULL, bsearch(&hi, v, n, sizeof *v, cmp_int) == NULL);
    }
    const char *needle = "fig";
    const char **fw = bsearch(&needle, w, sizeof w / sizeof *w, sizeof *w, cmp_str);
    printf("bsearch word=%s\n", fw ? *fw : "(none)");
    return 0;
}
