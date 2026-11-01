/* printf, C17 7.21.6.1 and 7.21.6.3, by hand: each conversion and flag
 * at its edges, the length modifiers, `*` width and precision, %n, and
 * the failures (an incomplete specification, %lc outside the C locale's
 * ASCII, a count past INT_MAX), each with its return value and errno.
 * Where C leaves the answer open glibc's is the reference
 * (docs/LEDGER.md). The floating conversions and positional arguments
 * are muzzle's own refusal (tests/own/), not here. */
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wformat-extra-args"
#pragma clang diagnostic ignored "-Wformat-invalid-specifier"
#pragma clang diagnostic ignored "-Wformat-zero-length"
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

static int line;

#define P(...)                                     \
    do {                                           \
        printf("%3d ", ++line);                    \
        errno = 0;                                 \
        int r_ = printf(__VA_ARGS__);              \
        int e_ = errno;                            \
        printf(" ->%d e=%d\n", r_, e_);            \
    } while (0)

int main(void) {
    void *z = (void *)0, *a = (void *)0x1234, *hi = (void *)0xffffffffffffffffUL;
    P("[%p]", z); P("[%p]", a); P("[%20p]", a); P("[%-20p]", a); P("[%+p]", a); P("[% p]", a);
    P("[%#p]", a); P("[%020p]", a); P("[%.8p]", a); P("[%10p]", z); P("[%-10p]", z); P("[%.1p]", z);
    P("[%+p]", z); P("[%010p]", z); P("[%.0p]", z); P("[%.0p]", a); P("[%p]", hi); P("[%-+25.20p]", a);

    const char *ns = 0;
    P("[%s]", ns); P("[%.3s]", ns); P("[%.6s]", ns); P("[%10s]", ns); P("[%.5s]", ns); P("[%-8.6s]", ns);
    P("[%.7s]", ns); P("[%s]", "wolf"); P("[%.2s]", "wolf"); P("[%6s]", "wolf"); P("[%-6s|]", "wolf");
    P("[%.0s]", "wolf"); P("[%s]", ""); P("[%5s]", ""); P("[%.*s]", 3, "abcdef"); P("[%.*s]", -1, "abc");
    char unterminated[3] = {'x', 'y', 'z'};
    P("[%.3s]", unterminated); P("[%.2s]", unterminated);

    P("[%#o]", 0); P("[%#.0o]", 0); P("[%.0d]", 0); P("[%#x]", 0); P("[%#.0x]", 0); P("[%#5.3o]", 8);
    P("[%#o]", 8); P("[%#.3o]", 8); P("[%#.3x]", 1); P("[%+.0d]", 0); P("[% .0d]", 0); P("[%5.0d]", 0);
    P("[%#.1o]", 0); P("[%#2o]", 0); P("[%#.0o]", 1); P("[%.0x]", 0); P("[%#.0X]", 0); P("[%-5.0u]|", 0u);

    P("[%05s]", "ab"); P("[%-05d]", 42); P("[% +d]", 42); P("[%+ d]", 42); P("[%hhd]", 300); P("[%hhu]", -1);
    P("[%hd]", 70000); P("[%5%]"); P("[%-5%]"); P("[%5c]", 'x'); P("[%-5c]", 'x'); P("[%.3c]", 'x');
    P("[%05c]", 'x'); P("[%c]", 256 + 65); P("[%+c]", 'x'); P("[%#c]", 'x'); P("[%c]", 0); P("[%3c]", -1);
    P("[%.3%]"); P("[%-#08.3x]", 10); P("[%+08d]", -5); P("[% 08d]", 5); P("[%08d]", -5); P("[%.10d]", -5);
    P("[%3.1d]", 12345); P("[%+u]", 5); P("[% x]", 5); P("[%+o]", 5); P("[%#d]", 5); P("[%#u]", 5);
    P("[%#s]", "ab"); P("[%-#8o]", 8); P("[%0-8d]", 8); P("[%-+5d]", 0); P("[%'d]", 1234567); P("[%'x]", 0xabcdef);

    P("[%lc]", (wint_t)0x41); P("[%ls]", L"ab"); P("[%5lc]", (wint_t)0x42); P("[%-4ls]", L"cd");
    P("[%.1ls]", L"ef"); P("[%C]", (wint_t)0x47); P("[%S]", L"hi"); P("[%lc]", (wint_t)0);
    P("[%ls]", (wchar_t *)0); P("[%.3ls]", (wchar_t *)0);

    P("[%0*d]", -5, 42); P("[%.*d]", -3, 42); P("[%*d]", -5, 42); P("[%*.*d]", 6, 4, -7);
    P("[%-*.*d]", 6, 4, -7); P("[%*d]", 0, 9); P("[%.*x]", 0, 0); P("[%*s]", 4, "s"); P("[%-*c]", 3, 'q');

    P("[%zd]", (ptrdiff_t)-1); P("[%jd]", INTMAX_MIN); P("[%td]", PTRDIFF_MIN); P("[%lld]", LLONG_MIN);
    P("[%llx]", ULLONG_MAX); P("[%#X]", 255); P("[%#08x]", 255); P("[%08.3x]", 255); P("[%zu]", SIZE_MAX);
    P("[%jx]", UINTMAX_MAX); P("[%ld]", LONG_MIN); P("[%lu]", ULONG_MAX); P("[%lo]", ULONG_MAX);
    P("[%#lo]", ULONG_MAX); P("[%hhx]", 0x1ff); P("[%hx]", 0x1ffff); P("[%hho]", 0x1ff); P("[%#hhX]", 0xab);
    P("[%zx]", (size_t)-1); P("[%tx]", (ptrdiff_t)-1); P("[%ji]", (intmax_t)-5); P("[%lld]", 0LL);
    P("[%hd]", -32768); P("[%hi]", 32767); P("[%d]", INT_MIN); P("[%d]", INT_MAX); P("[%u]", UINT_MAX);
    P("[%x]", UINT_MAX); P("[%X]", 0xdeadbeef); P("[%o]", UINT_MAX); P("[%i]", -1); P("[%llu]", 0ULL);
    P("[%qd]", 5LL); P("[%Lx]", 5LL); P("[%Zu]", (size_t)7); P("[%llc]", 'x'); P("[%hs]", "ab"); P("[%zc]", 'q');

    P("[%k]"); P("[%y%d]", 5); P("[%hh]"); P("[%l]"); P("[%lh]", 5); P("[%hl]", 5); P("[%Z]");
    P("%s", ""); P(""); P("no conversions at all"); P("%%%%%c%%", 'w');

    int n1 = -1;
    signed char n2 = -1;
    short n3 = -1;
    long n4 = -1;
    long long n5 = -1;
    size_t n6 = 99;
    intmax_t n7 = -1;
    ptrdiff_t n8 = -1;
    P("abc%n", &n1); printf("n=%d\n", n1);
    P("ab%hhn%hn%ln%lln%zn%jn%tn", &n2, &n3, &n4, &n5, &n6, &n7, &n8);
    printf("n=%d %d %ld %lld %zu %jd %td\n", n2, n3, n4, n5, n6, n7, n8);
    P("[%5n]", &n1); printf("n=%d\n", n1);
    P("%300d%hhn", 1, &n2); printf("n=%d\n", n2);

    /* failures: the call writes none of its output */
    P("[%"); printf("\n"); P("[%5"); printf("\n"); P("[%-"); printf("\n"); P("[%h"); printf("\n");
    P("[%."); printf("\n"); P("[%.*", 3); printf("\n"); P("abc%lcdef", (wint_t)0xe9); printf("\n");
    P("[%ls]", L"a\xe9"); printf("\n"); P("[%lc]", (wint_t)0x80); printf("\n"); P("[%lc]", WEOF); printf("\n");
    errno = 0;
    int big = snprintf(NULL, 0, "%2147483646d", 1);
    printf("big.1 %d e=%d\n", big, errno);
    errno = 0;
    big = snprintf(NULL, 0, "%2147483647d", 1);
    printf("big.2 %d e=%d\n", big, errno);
    errno = 0;
    big = snprintf(NULL, 0, "%2147483647d%d", 1, 1);
    printf("big.3 %d e=%d\n", big, errno);
    errno = 0;
    big = snprintf(NULL, 0, "%2147483648d", 1);
    printf("big.4 %d e=%d\n", big, errno);
    errno = 0;
    big = snprintf(NULL, 0, "%.2147483648d", 1);
    printf("big.5 %d e=%d\n", big, errno);
    errno = 0;
    big = snprintf(NULL, 0, "%*d", INT_MIN, 1);
    printf("big.6 %d e=%d\n", big, errno);
    errno = 0;
    big = snprintf(NULL, 0, "%99999999999999999999d", 1);
    printf("big.7 %d e=%d\n", big, errno);
    return 0;
}
