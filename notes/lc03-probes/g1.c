/* lc03 g1: glibc as a black box, for the choices the standards leave
 * open. Built against the host glibc only. */
#define _GNU_SOURCE
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct rec { int key, seq; };
static int bykey(const void *a, const void *b) {
    const struct rec *x = a, *y = b;
    return (x->key > y->key) - (x->key < y->key);
}
static void stable(int n, int mod) {
    struct rec *r = malloc(sizeof *r * (size_t)n);
    unsigned s = 12345;
    for (int i = 0; i < n; i++) { s = s * 1103515245u + 12345u; r[i].key = (int)((s >> 16) % (unsigned)mod); r[i].seq = i; }
    qsort(r, (size_t)n, sizeof *r, bykey);
    int ok = 1;
    for (int i = 1; i < n; i++) if (r[i].key == r[i-1].key && r[i].seq < r[i-1].seq) ok = 0;
    printf("qsort.stable n=%d mod=%d %d\n", n, mod, ok);
    free(r);
}
static int hits;
static void h(void) { hits++; }
static void td(const char *s) {
    char *e; errno = 0;
    double d = strtod(s, &e);
    unsigned long long b; memcpy(&b, &d, 8);
    printf("strtod [%s] bits=%016llx end=%ld errno=%d\n", s, b, (long)(e - s), errno);
}
int main(void) {
    stable(5, 2); stable(40, 3); stable(1000, 7); stable(100000, 10); stable(2000000, 10);
    for (int i = -2; i <= 136; i++) printf("strerror %d [%s]\n", i, strerror(i));
    printf("strerror 1000 [%s]\nstrerror INT_MIN [%s]\n", strerror(1000), strerror(INT_MIN));
    errno = 0; perror("p"); errno = EINVAL; perror(""); perror(NULL);
    const char *ds[] = {"1e-320", "4.9e-324", "2.4703282292062327e-324", "2.4703282292062328e-324", "1e-400", "1e400",
        "0x1p-1074", "0x1p-1075", "0x1.8p-1075", "0x1.fffffffffffff8p1023", "nan(123)", "-nan", "nan(", "infinity", "INF", "infinit",
        "2.2250738585072011e-308", "2.2250738585072014e-308", "0x", "0x.p1", "1e", "1e+", ".", "-.5e-1", "  +12", "0x1P", "1.7976931348623158e308", "1.7976931348623159e308"};
    for (size_t i = 0; i < sizeof ds / sizeof *ds; i++) td(ds[i]);
    const char *ls[] = {"  -0x1Z", "0x", "08", "z", "  +", "-9223372036854775809", "18446744073709551616", "-1"};
    int bases[] = {0, 1, 37, 16, 36};
    for (size_t i = 0; i < sizeof ls / sizeof *ls; i++)
        for (size_t j = 0; j < 5; j++) {
            char *e; errno = 0; long v = strtol(ls[i], &e, bases[j]);
            errno = errno; int e1 = errno; errno = 0; unsigned long u = strtoul(ls[i], NULL, bases[j]);
            printf("strtol [%s] base=%d v=%ld end=%ld errno=%d ul=%lu errno=%d\n", ls[i], bases[j], v, (long)(e - ls[i]), e1, u, errno);
        }
    printf("RAND_MAX=%d rand=%d,%d,%d\n", RAND_MAX, rand(), rand(), rand());
    void *p = malloc(10); errno = 0; void *q = realloc(p, 0);
    printf("realloc0 null=%d errno=%d\n", q == NULL, errno);
    errno = 0; p = aligned_alloc(3, 10); printf("aligned_alloc(3) null=%d errno=%d\n", p == NULL, errno);
    errno = 0; p = aligned_alloc(16, 10); printf("aligned_alloc(16,10) null=%d errno=%d\n", p == NULL, errno);
    errno = 0; int r = posix_memalign(&p, 4, 10); printf("posix_memalign(4) r=%d errno=%d\n", r, errno);
    int n = 0; for (int i = 0; i < 100; i++) if (atexit(h) == 0) n++;
    printf("atexit ok=%d\n", n);
    time_t t0 = 0; struct tm g, l; gmtime_r(&t0, &g); localtime_r(&t0, &l);
    char buf[200];
    strftime(buf, sizeof buf, "%Z|%z|%c|%x|%X|%r|%p|%Ec|%Ox|%G-%V|%s|%k|%l|%P|%+", &g);
    printf("strftime gm [%s] zone=%s off=%ld\n", buf, g.tm_zone, g.tm_gmtoff);
    strftime(buf, sizeof buf, "%Z|%z", &l);
    printf("strftime local [%s] zone=%s off=%ld isdst=%d TZ=%s\n", buf, l.tm_zone, l.tm_gmtoff, l.tm_isdst, getenv("TZ") ? getenv("TZ") : "(unset)");
    struct tm m = {0}; m.tm_year = 70; m.tm_mday = 1; m.tm_isdst = 1;
    printf("mktime isdst=1 -> %ld isdst=%d\n", (long)mktime(&m), m.tm_isdst);
    m = (struct tm){0}; m.tm_year = 70; m.tm_mday = 1; m.tm_isdst = -1;
    printf("mktime isdst=-1 -> %ld isdst=%d zone=%s\n", (long)mktime(&m), m.tm_isdst, m.tm_zone);
    struct tm z = {0}; z.tm_year = 124; z.tm_mday = 3; z.tm_zone = NULL;
    strftime(buf, sizeof buf, "[%Z][%z]", &z);
    printf("strftime zero tm %s\n", buf);
    printf("CLOCKS_PER_SEC=%ld\n", (long)CLOCKS_PER_SEC);
    return 0;
}
