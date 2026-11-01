/* The printf corpus, generated from the conversion grammar of C17
 * 7.21.6.1: `%` [flags] [width] [.precision] [length] conversion. Every
 * subset of the five flags `- + space # 0` (32), widths none, 1, 6, 21,
 * `*` = 12 and `*` = -9 (6), precisions none, `.`, .0, .1, .4, .23,
 * `.*` = 5 and `.*` = -2 (8), every length of the integer conversions
 * (none hh h l ll j z t: 8), the conversions d i u o x X (6), and eight
 * value edges per length: 589,824 calls. Then c, s, p, % and n over the
 * same flags, widths and precisions, with their lengths (l for c and s).
 * Each line is the format, the output between bars, and the return.
 * Arguments are passed at their exact C types. */
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#pragma clang diagnostic ignored "-Wformat-security"
#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

static const char *widths[] = {"", "1", "6", "21", "*", "*"};
static const int width_star[] = {0, 0, 0, 0, 12, -9};
static const char *precs[] = {"", ".", ".0", ".1", ".4", ".23", ".*", ".*"};
static const int prec_star[] = {0, 0, 0, 0, 0, 0, 5, -2};
enum { NONE, HH, H, L, LL, J, Z, T, NLEN };
static const char *lens[] = {"", "hh", "h", "l", "ll", "j", "z", "t"};
static const char convs[] = "diuoxX";

static const long long vals[NLEN][8] = {
    {0, 1, -1, 42, INT_MAX, INT_MIN, 255, 65543},
    {0, 1, -1, 127, -128, 255, 300, -300},
    {0, 1, -1, 32767, -32768, 65535, 70000, -70000},
    {0, 1, -1, LONG_MAX, LONG_MIN, 4294967296LL, -4294967297LL, 1000000007},
    {0, 1, -1, LLONG_MAX, LLONG_MIN, 4294967296LL, -4294967297LL, 1000000007},
    {0, 1, -1, INTMAX_MAX, INTMAX_MIN, 4294967296LL, -4294967297LL, 1000000007},
    {0, 1, -1, PTRDIFF_MAX, PTRDIFF_MIN, 4294967296LL, -4294967297LL, 1000000007},
    {0, 1, -1, PTRDIFF_MAX, PTRDIFF_MIN, 4294967296LL, -4294967297LL, 1000000007},
};

static char fmt[64];
static long calls;

/* fmt = "%" flags width precision length conv */
static void build(int flags, int w, int p, const char *len, char conv) {
    char *o = fmt;
    *o++ = '%';
    const char *fl = "-+ #0";
    for (int i = 0; i < 5; i++)
        if (flags & (1 << i))
            *o++ = fl[i];
    for (const char *s = widths[w]; *s; s++)
        *o++ = *s;
    for (const char *s = precs[p]; *s; s++)
        *o++ = *s;
    for (const char *s = len; *s; s++)
        *o++ = *s;
    *o++ = conv;
    *o = 0;
}

/* One call: the format, then the output between bars, then the return.
 * The width and precision `*` arguments come first, as C17 7.21.6.1p5
 * orders them. */
#define CALL(T, v)                                                               \
    do {                                                                         \
        int r_;                                                                  \
        printf("%s|", fmt);                                                      \
        if (ws && ps)                                                            \
            r_ = printf(fmt, width_star[w], prec_star[p], (T)(v));               \
        else if (ws)                                                             \
            r_ = printf(fmt, width_star[w], (T)(v));                             \
        else if (ps)                                                             \
            r_ = printf(fmt, prec_star[p], (T)(v));                              \
        else                                                                     \
            r_ = printf(fmt, (T)(v));                                            \
        printf("|%d\n", r_);                                                     \
        calls++;                                                                 \
    } while (0)

static void integers(void) {
    for (int c = 0; convs[c]; c++) {
        int is_signed = convs[c] == 'd' || convs[c] == 'i';
        for (int len = 0; len < NLEN; len++)
            for (int flags = 0; flags < 32; flags++)
                for (int w = 0; w < 6; w++)
                    for (int p = 0; p < 8; p++) {
                        build(flags, w, p, lens[len], convs[c]);
                        int ws = width_star[w] != 0, ps = prec_star[p] != 0;
                        for (int k = 0; k < 8; k++) {
                            long long v = vals[len][k];
                            switch (len) {
                            case NONE:
                            case HH:
                            case H:
                                if (is_signed)
                                    CALL(int, v);
                                else
                                    CALL(unsigned int, v);
                                break;
                            case L:
                                if (is_signed)
                                    CALL(long, v);
                                else
                                    CALL(unsigned long, v);
                                break;
                            case LL:
                                if (is_signed)
                                    CALL(long long, v);
                                else
                                    CALL(unsigned long long, v);
                                break;
                            case J:
                                if (is_signed)
                                    CALL(intmax_t, v);
                                else
                                    CALL(uintmax_t, v);
                                break;
                            case Z:
                                if (is_signed)
                                    CALL(ptrdiff_t, v);
                                else
                                    CALL(size_t, v);
                                break;
                            default:
                                if (is_signed)
                                    CALL(ptrdiff_t, v);
                                else
                                    CALL(size_t, v);
                                break;
                            }
                        }
                    }
    }
}

static void others(void) {
    static const int chars[] = {'a', 0, 255, -1, 'Z', 0x141};
    static const wint_t wchars[] = {'a', 0, 127, 'Z'};
    static const char *strs[] = {"", "a", "wolf", "a longer string of text", 0};
    static const wchar_t *wstrs[] = {L"", L"a", L"wolf", L"a longer wide string", 0};
    static void *ptrs[5];
    ptrs[0] = 0;
    ptrs[1] = (void *)1;
    ptrs[2] = (void *)0x1234;
    ptrs[3] = (void *)0xdeadbeefUL;
    ptrs[4] = (void *)0xffffffffffffffffUL;
    for (int flags = 0; flags < 32; flags++)
        for (int w = 0; w < 6; w++)
            for (int p = 0; p < 8; p++) {
                int ws = width_star[w] != 0, ps = prec_star[p] != 0;
                build(flags, w, p, "", 'c');
                for (int k = 0; k < 6; k++)
                    CALL(int, chars[k]);
                build(flags, w, p, "l", 'c');
                for (int k = 0; k < 4; k++)
                    CALL(wint_t, wchars[k]);
                build(flags, w, p, "", 's');
                for (int k = 0; k < 5; k++)
                    CALL(const char *, strs[k]);
                build(flags, w, p, "l", 's');
                for (int k = 0; k < 5; k++)
                    CALL(const wchar_t *, wstrs[k]);
                build(flags, w, p, "", 'p');
                for (int k = 0; k < 5; k++)
                    CALL(void *, ptrs[k]);
                build(flags, w, p, "", '%');
                {
                    int r_;
                    printf("%s|", fmt);
                    if (ws && ps)
                        r_ = printf(fmt, width_star[w], prec_star[p]);
                    else if (ws)
                        r_ = printf(fmt, width_star[w]);
                    else if (ps)
                        r_ = printf(fmt, prec_star[p]);
                    else
                        r_ = printf(fmt);
                    printf("|%d\n", r_);
                    calls++;
                }
            }
    /* %n after a prefix of each length, at every length modifier */
    static const char *prefixes[] = {"", "x", "%5d", "%-300d"};
    for (int k = 0; k < 4; k++) {
        signed char a;
        short b;
        int c;
        long d;
        long long e;
        intmax_t f;
        size_t g;
        ptrdiff_t h;
        char nf[80];
        strcpy(nf, prefixes[k]);
        size_t at = strlen(nf);
        memcpy(nf + at, "%hhn%hn%n%ln%lln%jn%zn%tn", 26);
        int r;
        if (k < 2)
            r = printf(nf, &a, &b, &c, &d, &e, &f, &g, &h);
        else
            r = printf(nf, 7, &a, &b, &c, &d, &e, &f, &g, &h);
        printf("|n %d: %d %d %d %ld %lld %jd %zu %td r=%d\n", k, a, b, c, d, e, f, g, h, r);
        calls++;
    }
    printf("calls=%ld\n", calls);
}

int main(void) {
    integers();
    others();
    return 0;
}
