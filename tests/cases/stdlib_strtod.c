/* <stdlib.h> C17 7.22.1.3 strtod: correctly rounded (C17 7.22.1.3p9 and
 * F.5, round to nearest, ties to even), hexadecimal forms exact or
 * rounded the same way, INF/INFINITY/NAN/NAN(n-char-sequence), ERANGE on
 * overflow and underflow, and the end pointer. A double is printed as its
 * bits (muzzle's printf has no floating conversions yet). The corpora
 * print as counts and a digest:
 *   - every exact halfway point between two neighbouring doubles of a
 *     sample, written out in full decimal by this program (up to ~770
 *     digits), plus the same just above and just below it;
 *   - random decimal strings of 1 to 25 digits over the whole exponent
 *     range, in several spellings;
 *   - random hexadecimal strings, rounded at 53 bits and in the
 *     subnormal range. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long long bits_of(double d) {
    unsigned long long b;
    memcpy(&b, &d, sizeof b);
    return b;
}

static void one(const char *s) {
    char *e = NULL;
    errno = 0;
    double d = strtod(s, &e);
    printf("[%s] %016llx end=%ld errno=%d\n", s, bits_of(d), (long)(e - s), errno);
}

static unsigned long long seed = 1;
static unsigned long long next64(void) {
    seed ^= seed << 13;
    seed ^= seed >> 7;
    seed ^= seed << 17;
    return seed;
}

/* A running digest of everything a corpus produced. */
static unsigned long long dg;
static long n_conv, n_erange;
static void absorb(const char *s, int want_end) {
    char *e = NULL;
    errno = 0;
    double d = strtod(s, &e);
    unsigned long long b = bits_of(d);
    n_conv++;
    n_erange += errno == ERANGE;
    dg ^= b + (unsigned long long)(e - s) * 0x9e3779b97f4a7c15ULL + (unsigned long long)errno;
    dg *= 0x100000001b3ULL;
    if (want_end >= 0 && e - s != want_end)
        printf("short end [%s] %ld\n", s, (long)(e - s));
}

/* Decimal digits, most significant first, of a big integer. */
static char dig[2400];
static int nd;

static void dec_set(unsigned long long v) {
    char t[24];
    int n = 0;
    do {
        t[n++] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    nd = n;
    for (int i = 0; i < n; i++)
        dig[i] = t[n - 1 - i];
}

static void dec_mul(unsigned m) {
    unsigned carry = 0;
    for (int i = nd - 1; i >= 0; i--) {
        unsigned x = (unsigned)(dig[i] - '0') * m + carry;
        dig[i] = (char)('0' + x % 10);
        carry = x / 10;
    }
    while (carry) {
        memmove(dig + 1, dig, (size_t)nd);
        dig[0] = (char)('0' + carry % 10);
        carry /= 10;
        nd++;
    }
}

static void dec_dec(void) {
    int i = nd - 1;
    while (dig[i] == '0') {
        dig[i] = '9';
        i--;
    }
    dig[i]--;
    if (dig[0] == '0' && nd > 1) {
        memmove(dig, dig + 1, (size_t)(nd - 1));
        nd--;
    }
}

static char buf[2600];

/* The exact decimal of (2m+1) * 2^(q), q may be negative, into buf with
 * `tail` appended to its last digit ("" for exact; "1" to go above;
 * the lower neighbour is made by the caller). */
static void midpoint(unsigned long long m2, int q, int below, const char *tail) {
    dec_set(m2);
    if (q >= 0) {
        for (int i = 0; i < q; i++)
            dec_mul(2);
        if (below) {
            dec_dec();
            snprintf(buf, sizeof buf, "%.*s.99999999", nd, dig);
        } else {
            snprintf(buf, sizeof buf, "%.*s%s", nd, dig, *tail ? ".00000000001" : "");
        }
        return;
    }
    int k = -q;
    for (int i = 0; i < k; i++)
        dec_mul(5);
    /* value = dig / 10^k */
    int n = nd;
    if (below)
        n--; /* the last digit is 5: dropping it goes below */
    if (nd > k)
        snprintf(buf, sizeof buf, "%.*s.%.*s%s", nd - k, dig, n - (nd - k), dig + (nd - k), tail);
    else
        snprintf(buf, sizeof buf, "0.%0*d%.*s%se0", k - nd, 0, n, dig, tail);
    if (k - nd == 0 && nd <= k) {
        /* "%0*d" with width 0 still prints one 0: rebuild without it */
        snprintf(buf, sizeof buf, "0.%.*s%se0", n, dig, tail);
    }
}

static void midpoints(void) {
    int cnt = 0;
    dg = 0; n_conv = 0; n_erange = 0;
    for (int e = -1074; e <= 971; e += 7) {
        for (int r = 0; r < 3; r++) {
            unsigned long long m;
            if (r == 0)
                m = (1ULL << 52) | (next64() >> 12);
            else if (r == 1)
                m = (1ULL << 53) - 1;
            else
                m = (1ULL << 52);
            /* d = m * 2^e; the midpoint above it is (2m+1) * 2^(e-1) */
            midpoint(2 * m + 1, e - 1, 0, "");
            absorb(buf, (int)strlen(buf));
            if (cnt < 6)
                one(buf);
            midpoint(2 * m + 1, e - 1, 0, "1");
            absorb(buf, (int)strlen(buf));
            midpoint(2 * m + 1, e - 1, 1, "");
            absorb(buf, (int)strlen(buf));
            cnt++;
        }
    }
    /* Subnormal neighbours: m * 2^-1074 for m < 2^52. */
    for (int i = 0; i < 400; i++) {
        unsigned long long m = i < 60 ? (unsigned long long)i : next64() >> (12 + (unsigned)(i % 52));
        midpoint(2 * m + 1, -1075, 0, "");
        absorb(buf, (int)strlen(buf));
        midpoint(2 * m + 1, -1075, 0, "1");
        absorb(buf, (int)strlen(buf));
        midpoint(2 * m + 1, -1075, 1, "");
        absorb(buf, (int)strlen(buf));
    }
    printf("midpoints conv=%ld erange=%ld digest=%016llx\n", n_conv, n_erange, dg);
}

static void random_decimal(void) {
    static const char *const spell[] = {"%s%.*s.%se%d", "%s%.*s%se%d", "%s0.%.*s%sE%+d", "%s000%.*s.%s0e%d", "%s%.*s%sE%d"};
    dg = 0; n_conv = 0; n_erange = 0;
    for (int i = 0; i < 200000; i++) {
        char digits[32];
        int len = 1 + (int)(next64() % 25);
        for (int k = 0; k < len; k++)
            digits[k] = (char)('0' + next64() % 10);
        digits[len] = 0;
        int split = (int)(next64() % (unsigned long long)(len + 1));
        int ex = (int)(next64() % 701) - 350;
        const char *sign = (i & 1) ? "-" : (i & 2) ? "+" : "";
        snprintf(buf, sizeof buf, spell[i % 5], sign, split, digits, digits + split, ex);
        absorb(buf, (int)strlen(buf));
        if (i < 5)
            one(buf);
    }
    printf("random decimal conv=%ld erange=%ld digest=%016llx\n", n_conv, n_erange, dg);
}

static void random_hex(void) {
    dg = 0; n_conv = 0; n_erange = 0;
    for (int i = 0; i < 50000; i++) {
        unsigned long long m = next64();
        int shift = (int)(next64() % 64);
        int ex = (int)(next64() % 2300) - 1150;
        const char *sign = (i & 1) ? "-" : "";
        if (i % 3 == 0)
            snprintf(buf, sizeof buf, "%s0x%llx.%llxp%d", sign, m >> shift, m & 0xffff, ex);
        else if (i % 3 == 1)
            snprintf(buf, sizeof buf, "%s0X%llXP%+d", sign, m, ex);
        else
            snprintf(buf, sizeof buf, "%s0x.%016llx%xp%d", sign, m, (unsigned)(m >> 40), ex);
        absorb(buf, (int)strlen(buf));
        if (i < 5)
            one(buf);
    }
    printf("random hex conv=%ld erange=%ld digest=%016llx\n", n_conv, n_erange, dg);
}

int main(void) {
    static const char *const t[] = {
        "0", "-0", "+0", "0.0", "1", "-1", "1.5", "  +12", "\t\n 3.25xyz", "0.1", "0.2", "0.3", "1e23", "8.5", "-.5e-1",
        "123456789012345678901234567890", "1.0000000000000002", "1.00000000000000011102230246251565404236316680908203125",
        "1.00000000000000011102230246251565404236316680908203124", "1.00000000000000011102230246251565404236316680908203126",
        "9007199254740993", "9007199254740995", "9007199254740993.0000000000000000000000000000000000000001",
        "4.9e-324", "5e-324", "2e-324", "2.4703282292062327e-324", "2.4703282292062328e-324", "1e-320", "1e-400",
        "2.2250738585072011e-308", "2.2250738585072014e-308", "2.2250738585072012e-308",
        "1.7976931348623157e308", "1.7976931348623158e308", "1.7976931348623159e308", "1e308", "1e309", "1e400", "-1e400",
        "1e", "1e+", "1e-", "1e+x", ".", "-.", "+.e1", "e5", "", " ", "-", "+", ".5", "5.", "5.e2", "00000.00001e5",
        "1e-2147483648", "1e2147483647", "1e99999999999999999999", "1e-99999999999999999999", "0e99999999999999999999",
        "0x", "0X", "0x.", "0x.p1", "0x1", "0x1p", "0x1P", "0x1p+", "0x1p3", "0xA.8p-1", "0x1.8p1", "0x.8", "-0x1p-2",
        "0x1p-1074", "0x1p-1075", "0x1.8p-1075", "0x1.0000000000001p-1074", "0x0.0000000000001p-1022",
        "0x1.fffffffffffffp1023", "0x1.fffffffffffff7ffp1023", "0x1.fffffffffffff8p1023", "0x1p1024", "0x123456789abcdef0123p0",
        "0x1.00000000000008p0", "0x1.00000000000008000001p0", "0x1.00000000000018p0", "0xg", "0x1.g",
        "inf", "INF", "-inf", "+Inf", "infinity", "INFINITY", "infinit", "infx", "in", "i",
        "nan", "NAN", "-nan", "+NaN", "nan()", "nan(123)", "nan(0x7ff)", "nan(abc_9)", "nan(", "nan(1", "nan(-)", "nanx", "na",
        "1,5", "1 5", "１",
    };
    for (size_t i = 0; i < sizeof t / sizeof *t; i++)
        one(t[i]);
    /* A null end pointer is allowed. */
    errno = 0;
    printf("null-end %016llx errno=%d\n", bits_of(strtod("2.5", NULL)), errno);
    /* 800 significant digits, then a nonzero digit far past the halfway point. */
    char *longs = malloc(4000);
    size_t at = 0;
    longs[at++] = '1';
    longs[at++] = '.';
    for (int i = 0; i < 800; i++)
        longs[at++] = '0';
    longs[at++] = '1';
    longs[at] = 0;
    one(longs);
    at = 0;
    longs[at++] = '0';
    longs[at++] = '.';
    for (int i = 0; i < 3000; i++)
        longs[at++] = i < 2990 ? '0' : '7';
    memcpy(longs + at, "e2700", 6);
    one(longs);
    free(longs);
    midpoints();
    random_decimal();
    random_hex();
    return 0;
}
