/* <stdlib.h> memory management, C17 7.22.3: malloc, calloc, realloc,
 * free. Addresses are never printed; alignment, contents and NULL-ness
 * are. */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "t.h"

#define N 600

static unsigned char *blk[N];
static size_t len[N];

static size_t size_for(int i) {
    /* small, medium and a few above the 64 KiB small-block ceiling */
    if (i % 97 == 0)
        return 70000 + (size_t)i * 13;
    return (size_t)((i * 37) % 700) + 1;
}

static int check(unsigned char *p, size_t n, unsigned char v) {
    for (size_t i = 0; i < n; i++)
        if (p[i] != v)
            return 0;
    return 1;
}

int main(void) {
    /* malloc(0): C17 lets it return NULL or a unique pointer; glibc
     * gives a pointer free accepts. */
    void *z = malloc(0);
    t_kv("malloc0.nonnull", z != NULL);
    free(z);
    free(NULL);
    t_kv("free.null", 1);

    /* Every block is aligned for any object (max_align_t, 16 on x86-64). */
    int aligned = 1;
    for (int i = 0; i < N; i++) {
        len[i] = size_for(i);
        blk[i] = malloc(len[i]);
        if (blk[i] == NULL || ((unsigned long)blk[i] % _Alignof(max_align_t)) != 0)
            aligned = 0;
        memset(blk[i], i & 0xff, len[i]);
    }
    t_kv("malloc.aligned", aligned);

    /* No two live blocks overlap: every block still holds its own byte. */
    int intact = 1;
    for (int i = 0; i < N; i++)
        intact &= check(blk[i], len[i], (unsigned char)(i & 0xff));
    t_kv("malloc.intact", intact);

    /* Free every other block, allocate again, and check the survivors. */
    for (int i = 0; i < N; i += 2) {
        free(blk[i]);
        blk[i] = NULL;
    }
    for (int i = 0; i < N; i += 2) {
        len[i] = size_for(N - i);
        blk[i] = malloc(len[i]);
        memset(blk[i], 0x5a, len[i]);
    }
    intact = 1;
    for (int i = 0; i < N; i++)
        intact &= check(blk[i], len[i], i % 2 == 0 ? 0x5a : (unsigned char)(i & 0xff));
    t_kv("malloc.reuse_intact", intact);
    for (int i = 0; i < N; i++)
        free(blk[i]);

    /* calloc zeroes, including memory that was used and freed. */
    unsigned char *d = malloc(4000);
    memset(d, 0xee, 4000);
    free(d);
    unsigned char *c = calloc(1000, 4);
    t_kv("calloc.nonnull", c != NULL);
    t_kv("calloc.zero", c != NULL && check(c, 4000, 0));
    free(c);
    c = calloc(3, 70000);
    t_kv("calloc.large_zero", c != NULL && check(c, 210000, 0));
    free(c);

    /* calloc whose product overflows size_t fails with ENOMEM. */
    errno = 0;
    c = calloc((size_t)-1 / 2 + 2, 2);
    t_kv("calloc.overflow_null", c == NULL);
    t_kv("calloc.overflow_errno_is_ENOMEM", errno == ENOMEM);

    /* A request no system can meet fails with ENOMEM. */
    errno = 0;
    void *huge = malloc((size_t)-1 / 2);
    t_kv("malloc.huge_null", huge == NULL);
    t_kv("malloc.huge_errno_is_ENOMEM", errno == ENOMEM);
    errno = 0;
    huge = malloc((size_t)-1 - 8);
    t_kv("malloc.max_null", huge == NULL);
    t_kv("malloc.max_errno_is_ENOMEM", errno == ENOMEM);

    /* realloc(NULL, n) is malloc(n). */
    unsigned char *r = realloc(NULL, 10);
    t_kv("realloc.null_nonnull", r != NULL);
    for (int i = 0; i < 10; i++)
        r[i] = (unsigned char)(i * 3);
    /* Growing keeps the old contents, through the small sizes and past
     * the small-block ceiling. */
    size_t sizes[] = {20, 100, 1000, 5000, 65536, 70000, 300000, 1 << 21};
    int kept = 1;
    size_t have = 10;
    for (size_t k = 0; k < sizeof sizes / sizeof sizes[0]; k++) {
        r = realloc(r, sizes[k]);
        if (r == NULL) {
            kept = 0;
            break;
        }
        for (size_t i = 0; i < have; i++)
            if (r[i] != (unsigned char)(i * 3))
                kept = 0;
        for (size_t i = have; i < sizes[k]; i++)
            r[i] = (unsigned char)(i * 3);
        have = sizes[k];
    }
    t_kv("realloc.grow_kept", kept);
    /* Shrinking keeps the prefix. */
    r = realloc(r, 33);
    kept = r != NULL;
    for (size_t i = 0; kept && i < 33; i++)
        if (r[i] != (unsigned char)(i * 3))
            kept = 0;
    t_kv("realloc.shrink_kept", kept);
    free(r);

    /* A failed realloc leaves the block alone and sets ENOMEM. */
    r = malloc(16);
    memset(r, 0x11, 16);
    errno = 0;
    unsigned char *q = realloc(r, (size_t)-1 / 2);
    t_kv("realloc.fail_null", q == NULL);
    t_kv("realloc.fail_errno_is_ENOMEM", errno == ENOMEM);
    t_kv("realloc.fail_kept", check(r, 16, 0x11));
    free(r);
    return 0;
}
