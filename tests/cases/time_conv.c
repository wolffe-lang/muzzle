/* <time.h> conversions: C17 7.27.3.3 gmtime, 7.27.3.4 localtime,
 * 7.27.2.3 mktime; POSIX.1-2024 gmtime_r, localtime_r, timegm, and
 * struct tm's tm_gmtoff and tm_zone. The local zone is UTC (the harness
 * sets TZ=UTC for both columns; muzzle has no zone database yet,
 * docs/LEDGER.md). mktime normalizes every field (C17 7.27.2.3p2) and
 * fails with EOVERFLOW when the result does not fit. */
#define _DEFAULT_SOURCE
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static void show(const char *what, const struct tm *t) {
    if (t == NULL) {
        printf("%s null\n", what);
        return;
    }
    printf("%s %d-%d-%d %d:%d:%d wday=%d yday=%d isdst=%d off=%ld zone=%s\n", what, t->tm_year, t->tm_mon, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec, t->tm_wday, t->tm_yday, t->tm_isdst, t->tm_gmtoff,
           t->tm_zone ? t->tm_zone : "(null)");
}

static unsigned long long dg;
static void mix(unsigned long long v) {
    dg ^= v;
    dg *= 0x100000001b3ULL;
}

int main(void) {
    static const long long ts[] = {
        0, -1, 1, 59, 60, 86399, 86400, -86400, -86401, 951782400, 951868800, 946684799, 978307200,
        -2208988800LL, -2203891200LL, 4107542400LL, 4107456000LL, 2147483647LL, 2147483648LL, -2147483648LL,
        -2147483649LL, 253402300799LL, 253402300800LL, -62135596800LL, -62135596801LL, -62167219200LL,
        -62167219201LL, 1700000000LL, 1709164800LL, 1735689599LL, 4294967295LL, 32503680000LL,
        67768036191676799LL, 67768036191676800LL, -67768040609740800LL, -67768040609740801LL,
        -67768040609744000LL, LLONG_MAX, LLONG_MIN,
    };
    for (size_t i = 0; i < sizeof ts / sizeof *ts; i++) {
        time_t t = (time_t)ts[i];
        struct tm g, l;
        memset(&g, 0x55, sizeof g);
        errno = 0;
        struct tm *gp = gmtime_r(&t, &g);
        int ge = errno;
        printf("t=%lld errno=%d ", ts[i], ge);
        show("gmtime_r", gp);
        memset(&l, 0x55, sizeof l);
        struct tm *lp = localtime_r(&t, &l);
        show("  localtime_r", lp);
        if (gp != NULL) {
            struct tm c = g;
            errno = 0;
            time_t back = timegm(&c);
            printf("  timegm=%lld errno=%d\n", (long long)back, errno);
            c = l;
            errno = 0;
            back = mktime(&c);
            printf("  mktime=%lld errno=%d\n", (long long)back, errno);
        }
    }
    /* The static results answer the same. */
    time_t t = 951782400;
    show("gmtime", gmtime(&t));
    show("localtime", localtime(&t));

    /* Every day across 1900..2100 and a few seconds in each, round trip. */
    int ok = 1;
    dg = 0;
    for (long long d = -25567; d < 47482; d++) {
        long long s = d * 86400 + (d * 7919) % 86400;
        time_t x = (time_t)s;
        struct tm g;
        if (gmtime_r(&x, &g) == NULL) {
            ok = 0;
            continue;
        }
        mix((unsigned long long)g.tm_year << 40 | (unsigned long long)g.tm_mon << 32 | (unsigned)g.tm_mday << 24 |
            (unsigned)g.tm_hour << 16 | (unsigned)g.tm_min << 8 | (unsigned)g.tm_sec);
        mix((unsigned)g.tm_wday << 16 | (unsigned)g.tm_yday);
        struct tm c = g;
        ok &= timegm(&c) == x;
        c = g;
        ok &= mktime(&c) == x;
    }
    printf("sweep ok=%d digest=%016llx\n", ok, dg);

    /* mktime and timegm normalize out-of-range fields; tm_wday and
     * tm_yday are ignored on input. */
    struct norm { int y, mon, mday, h, m, s, isdst; };
    static const struct norm n[] = {
        {124, 13, 1, 0, 0, 0, 0}, {124, 0, 0, 0, 0, 0, 0}, {124, 2, 0, 0, 0, 0, 0}, {123, 2, 0, 0, 0, 0, 0},
        {124, 0, -1, 0, 0, 0, 0}, {124, 0, 400, 0, 0, 0, 0}, {124, 5, 15, 0, 0, -1, 0}, {124, 5, 15, 0, 0, 60, 0},
        {124, 5, 15, 0, 0, 108000, 0}, {124, 5, 15, 0, -61, 0, 0}, {124, 5, 15, 25, 0, 0, 0}, {124, 5, 15, -1, 0, 0, 0},
        {124, -1, 15, 0, 0, 0, 0}, {124, -13, 15, 0, 0, 0, 0}, {123, 1, 29, 12, 0, 0, 0}, {100, 1, 29, 0, 0, 0, 0},
        {0, 1, 29, 0, 0, 0, 0}, {70, 0, 1, 0, 0, 0, 1}, {70, 0, 1, 0, 0, 0, -1}, {70, 0, 1, 0, 0, 0, 5},
        {70, 0, 1, 0, 0, -1, 0}, {69, 11, 31, 23, 59, 59, 0}, {-1900, 0, 1, 0, 0, 0, 0}, {-2000, 0, 1, 0, 0, 0, 0},
        {1100, 0, 1, 0, 0, 0, 0}, {8099, 11, 31, 23, 59, 59, 0}, {124, 1200, 1, 0, 0, 0, 0}, {124, -1200, 1, 0, 0, 0, 0},
        {124, 0, 1, 0, 0, INT_MAX, 0}, {124, 0, 1, 0, INT_MIN, 0, 0}, {124, 0, INT_MAX, 0, 0, 0, 0},
        {INT_MAX, 11, 31, 23, 59, 59, 0}, {INT_MAX, 12, 1, 0, 0, 0, 0}, {INT_MIN, 0, 1, 0, 0, 0, 0},
        {INT_MIN + 1900, 0, 1, 0, 0, 0, 0}, {2147481747, 11, 31, 23, 59, 59, 0},
    };
    for (size_t i = 0; i < sizeof n / sizeof *n; i++) {
        struct tm a;
        memset(&a, 0, sizeof a);
        a.tm_year = n[i].y; a.tm_mon = n[i].mon; a.tm_mday = n[i].mday;
        a.tm_hour = n[i].h; a.tm_min = n[i].m; a.tm_sec = n[i].s; a.tm_isdst = n[i].isdst;
        a.tm_wday = 99; a.tm_yday = -5;
        struct tm b = a;
        errno = 0;
        time_t r = mktime(&a);
        printf("mktime #%zu=%lld errno=%d ", i, (long long)r, errno);
        show("", &a);
        errno = 0;
        r = timegm(&b);
        printf("timegm #%zu=%lld errno=%d ", i, (long long)r, errno);
        show("", &b);
    }
    return 0;
}
