/* <stdlib.h> aligned allocation and realloc's growth: C17 7.22.3.1
 * aligned_alloc, 7.22.3.5 realloc, POSIX.1-2024 posix_memalign.
 * Addresses are never printed: alignment, contents and NULL-ness are.
 * realloc must keep every byte of the old block, whichever way it grows
 * (in place or by a copy), across the allocator's small classes, the
 * mapped blocks and aligned blocks. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fill(unsigned char *p, size_t from, size_t to, unsigned seed) {
    for (size_t i = from; i < to; i++)
        p[i] = (unsigned char)(i * 31u + seed);
}

static int kept(const unsigned char *p, size_t n, unsigned seed) {
    for (size_t i = 0; i < n; i++)
        if (p[i] != (unsigned char)(i * 31u + seed))
            return 0;
    return 1;
}

static int is_aligned(const void *p, size_t a) { return ((uintptr_t)p % a) == 0; }

int main(void) {
    static const size_t aligns[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 1024, 4096, 65536, 1u << 20};
    static const size_t sizes[] = {1, 16, 100, 4096, 70000};
    for (size_t i = 0; i < sizeof aligns / sizeof *aligns; i++) {
        for (size_t j = 0; j < sizeof sizes / sizeof *sizes; j++) {
            size_t a = aligns[i], n = sizes[j];
            errno = 0;
            unsigned char *p = aligned_alloc(a, n);
            int ok = p != NULL && is_aligned(p, a);
            if (p != NULL) {
                fill(p, 0, n, (unsigned)(i + j));
                ok &= kept(p, n, (unsigned)(i + j));
            }
            printf("aligned_alloc(%zu,%zu) null=%d ok=%d errno=%d\n", a, n, p == NULL, ok, errno);
            free(p);
        }
    }
    /* Not a power of two: C17 7.22.3.1p2 (a null pointer for an
     * alignment the implementation does not support). */
    static const size_t bad[] = {0, 3, 24, 100};
    for (size_t i = 0; i < sizeof bad / sizeof *bad; i++) {
        errno = 0;
        void *p = aligned_alloc(bad[i], 64);
        printf("aligned_alloc(%zu,64) null=%d errno=%d\n", bad[i], p == NULL, errno);
        free(p);
    }
    errno = 0;
    void *huge = aligned_alloc(64, SIZE_MAX - 4096);
    printf("aligned_alloc(huge) null=%d errno=%d\n", huge == NULL, errno);

    /* posix_memalign answers an error number and leaves errno alone. */
    static const size_t pm[] = {8, 16, 32, 4096, 65536, 0, 4, 12, 24};
    for (size_t i = 0; i < sizeof pm / sizeof *pm; i++) {
        void *p = (void *)&huge;
        errno = 0;
        int r = posix_memalign(&p, pm[i], 200);
        int ok = r == 0 && p != NULL && is_aligned(p, pm[i]);
        if (r == 0) {
            memset(p, 0x5a, 200);
            free(p);
        }
        printf("posix_memalign(%zu,200) r=%d ok=%d untouched=%d errno=%d\n", pm[i], r, ok, r != 0 && p == (void *)&huge, errno);
    }
    {
        void *p = NULL;
        int r = posix_memalign(&p, 64, 0);
        printf("posix_memalign(64,0) r=%d\n", r);
        if (r == 0)
            free(p);
        errno = 0;
        r = posix_memalign(&p, 64, SIZE_MAX - 4096);
        printf("posix_memalign(64,huge) r=%d errno=%d\n", r, errno);
    }

    /* realloc's growth keeps every byte, through every class and onto
     * mapped blocks; then shrinks back keeping the prefix. */
    static const size_t steps[] = {1, 7, 16, 17, 31, 100, 1000, 4096, 4097, 65536, 65537, 200000, 1u << 20, 3u << 20};
    size_t ns = sizeof steps / sizeof *steps;
    unsigned char *p = realloc(NULL, steps[0]);
    fill(p, 0, steps[0], 7);
    for (size_t i = 1; i < ns; i++) {
        unsigned char *q = realloc(p, steps[i]);
        int k = q != NULL && kept(q, steps[i - 1], 7);
        printf("realloc %zu->%zu kept=%d\n", steps[i - 1], steps[i], k);
        if (q == NULL)
            return 1;
        fill(q, steps[i - 1], steps[i], 7);
        p = q;
    }
    for (size_t i = ns - 1; i > 0; i--) {
        unsigned char *q = realloc(p, steps[i - 1]);
        printf("realloc %zu->%zu kept=%d\n", steps[i], steps[i - 1], q != NULL && kept(q, steps[i - 1], 7));
        if (q == NULL)
            return 1;
        p = q;
    }
    /* A failed realloc leaves the block as it was (C17 7.22.3.5p3). */
    errno = 0;
    unsigned char *q = realloc(p, SIZE_MAX - 4096);
    printf("realloc huge null=%d errno=%d still=%d\n", q == NULL, errno, kept(p, steps[0], 7));
    free(p);

    /* Aligned blocks grow by realloc like any other, keeping bytes. */
    for (size_t a = 32; a <= 8192; a *= 4) {
        unsigned char *b = aligned_alloc(a, a);
        fill(b, 0, a, 3);
        unsigned char *c = realloc(b, a * 50);
        printf("realloc aligned(%zu) %zu->%zu kept=%d\n", a, a, a * 50, c != NULL && kept(c, a, 3));
        unsigned char *d = realloc(c, 10);
        printf("realloc aligned(%zu) shrink kept=%d\n", a, d != NULL && kept(d, 10, 3));
        free(d);
    }

    /* Interleaved: many aligned and plain blocks live at once, each
     * still holding its own bytes after the others are freed. */
    enum { N = 300 };
    static unsigned char *blk[N];
    static size_t len[N];
    int intact = 1, aligned = 1;
    for (int i = 0; i < N; i++) {
        len[i] = (size_t)(i * 53 % 900) + 1;
        size_t a = (size_t)1 << (i % 9 + 4);
        blk[i] = i % 3 == 0 ? malloc(len[i]) : aligned_alloc(a, len[i]);
        if (blk[i] == NULL || (i % 3 != 0 && !is_aligned(blk[i], a)))
            aligned = 0;
        else
            fill(blk[i], 0, len[i], (unsigned)i);
    }
    for (int i = 0; i < N; i += 2) {
        free(blk[i]);
        blk[i] = NULL;
    }
    for (int i = 1; i < N; i += 2)
        intact &= blk[i] != NULL && kept(blk[i], len[i], (unsigned)i);
    for (int i = 1; i < N; i += 2) {
        unsigned char *g = realloc(blk[i], len[i] * 3);
        intact &= g != NULL && kept(g, len[i], (unsigned)i);
        free(g);
    }
    printf("mixed aligned=%d intact=%d\n", aligned, intact);

    /* calloc's product overflow is ENOMEM; its block is zeroed. */
    errno = 0;
    void *z = calloc(SIZE_MAX / 2, 3);
    printf("calloc overflow null=%d errno=%d\n", z == NULL, errno);
    unsigned char *c = calloc(1000, 70);
    int zero = c != NULL;
    for (size_t i = 0; c != NULL && i < 70000; i++)
        zero &= c[i] == 0;
    printf("calloc zeroed=%d\n", zero);
    free(c);
    return 0;
}
