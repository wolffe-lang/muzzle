/* g1: glibc as a black box, the printf corners the standard leaves open
 * or that are easy to get wrong. Built against glibc only (a probe). */
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wformat-extra-args"
#pragma clang diagnostic ignored "-Wformat-invalid-specifier"
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <wchar.h>

#define P(...) do { errno = 0; int r_ = printf(__VA_ARGS__); int e_ = errno; printf(" ->%d e=%d\n", r_, e_); } while (0)

int main(void) {
    void *z = (void *)0, *a = (void *)0x1234;
    P("%d [%p]", 1, z); P("%d [%p]", 2, a); P("%d [%20p]", 3, a); P("%d [%-20p]", 4, a);
    P("%d [%+p]", 5, a); P("%d [% p]", 6, a); P("%d [%#p]", 7, a); P("%d [%020p]", 8, a);
    P("%d [%.8p]", 9, a); P("%d [%10p]", 10, z); P("%d [%-10p]", 11, z); P("%d [%.1p]", 12, z);
    P("%d [%+p]", 13, z); P("%d [%010p]", 14, z); P("%d [%.0p]", 15, z); P("%d [%.0p]", 16, a);
    const char *ns = 0;
    P("%d [%s]", 20, ns); P("%d [%.3s]", 21, ns); P("%d [%.6s]", 22, ns); P("%d [%10s]", 23, ns);
    P("%d [%.5s]", 24, ns); P("%d [%-8.6s]", 25, ns); P("%d [%.7s]", 26, ns);
    P("%d [%#o]", 30, 0); P("%d [%#.0o]", 31, 0); P("%d [%.0d]", 32, 0); P("%d [%#x]", 33, 0);
    P("%d [%#.0x]", 34, 0); P("%d [%#5.3o]", 35, 8); P("%d [%#o]", 36, 8); P("%d [%#.3o]", 37, 8);
    P("%d [%#.3x]", 38, 1); P("%d [%+.0d]", 39, 0); P("%d [% .0d]", 40, 0); P("%d [%5.0d]", 41, 0);
    P("%d [%05s]", 50, "ab"); P("%d [%-05d]", 51, 42); P("%d [% +d]", 52, 42); P("%d [%+ d]", 53, 42);
    P("%d [%hhd]", 54, 300); P("%d [%hhu]", 55, -1); P("%d [%hd]", 56, 70000); P("%d [%5%]", 57);
    P("%d [%-5%]", 58); P("%d [%5c]", 59, 'x'); P("%d [%-5c]", 60, 'x'); P("%d [%.3c]", 61, 'x');
    P("%d [%05c]", 62, 'x'); P("%d [%c]", 63, 256 + 65); P("%d [%+c]", 64, 'x'); P("%d [%#c]", 65, 'x');
    P("%d [%lc]", 70, (wint_t)0x41); P("%d [%lc]", 71, (wint_t)0xe9); P("%d [%ls]", 72, L"ab");
    P("%d [%ls]", 73, L"a\xe9"); P("%d [%5lc]", 74, (wint_t)0x42);
    P("%d [%0*d]", 80, -5, 42); P("%d [%.*d]", 81, -3, 42); P("%d [%*d]", 82, -5, 42);
    P("%d [%*.*d]", 83, 6, 4, -7); P("%d [%-*.*d]", 84, 6, 4, -7);
    P("%d [%zd]", 90, (ptrdiff_t)-1); P("%d [%jd]", 91, INTMAX_MIN); P("%d [%td]", 92, PTRDIFF_MIN);
    P("%d [%lld]", 93, LLONG_MIN); P("%d [%llx]", 94, ULLONG_MAX); P("%d [%#X]", 95, 255);
    P("%d [%#08x]", 96, 255); P("%d [%08.3x]", 97, 255); P("%d [%zu]", 98, SIZE_MAX); P("%d [%jx]", 99, UINTMAX_MAX);
    P("%d [%ld]", 100, LONG_MIN); P("%d [%lu]", 101, ULONG_MAX); P("%d [%lo]", 102, ULONG_MAX); P("%d [%#lo]", 103, ULONG_MAX);
    P("%d [%hhx]", 104, 0x1ff); P("%d [%hx]", 105, 0x1ffff); P("%d [%hho]", 106, 0x1ff); P("%d [%#hhX]", 107, 0xab);
    P("%d [%zx]", 108, (size_t)-1); P("%d [%tx]", 109, (ptrdiff_t)-1); P("%d [%ji]", 110, (intmax_t)-5);
    P("%d [%k]", 120); P("%d [%y%d]", 121, 5); P("%d [%1$d %1$d]", 122); P("%d [%'d]", 123, 1234567);
    P("%d [%-+5d]", 124, 0); P("%d [%lld]", 125, 0LL); P("%d [%hd]", 126, -32768); P("%d [%#.0X]", 127, 0);
    P("%d [%Lx]", 128, 5); P("%d [%llc]", 129, 'x'); P("%d [%hs]", 130, "ab"); P("%d [%zc]", 131, 'q');
    P("%d [%+u]", 132, 5); P("%d [% x]", 133, 5); P("%d [%+o]", 134, 5); P("%d [%#d]", 135, 5); P("%d [%#u]", 136, 5);
    P("%d [%#s]", 137, "ab"); P("%d [%-#8o]", 138, 8); P("%d [%0-8d]", 139, 8);
    P("%d [%.3%]", 140); P("%d [%-#08.3x]", 141, 10); P("%d [%+08d]", 142, -5); P("%d [% 08d]", 143, 5);
    P("%d [%08d]", 144, -5); P("%d [%.10d]", 145, -5); P("%d [%3.1d]", 146, 12345); P("%d [%.0x]", 147, 0);
    P("%d [%#.1o]", 148, 0); P("%d [%#2o]", 149, 0); P("%d [%#.0o]", 150, 1);
    P("%d [%", 160); printf("\n"); P("%d [%5", 161); printf("\n"); P("%d [%-", 162); printf("\n");
    P("%d [%h", 163); printf("\n"); P("%d [%.", 164); printf("\n"); P("%d [%.*", 165, 3); printf("\n");
    P("%d [%hh]", 166); P("%d [%l]", 167); P("%d [%lh]", 168, 5); P("%d [%hl]", 169, 5);
    P("%d [%qd]", 170, 5LL); P("%d [%Z]", 171); P("%d [%m]", 172); P("%d [%C]", 173, (wint_t)0x41); P("%d [%S]", 174, L"ab");
    int n1 = -1; signed char n2 = -1; short n3 = -1; long n4 = -1; long long n5 = -1; size_t n6 = 99; intmax_t n7 = -1; ptrdiff_t n8 = -1;
    P("%d abc%n", 180, &n1); printf("n=%d\n", n1);
    P("%d ab%hhn%hn%ln%lln%zn%jn%tn", 181, &n2, &n3, &n4, &n5, &n6, &n7, &n8);
    printf("n=%d %d %ld %lld %zu %jd %td\n", n2, n3, n4, n5, n6, n7, n8);
    P("%d [%5n]", 182, &n1); printf("n=%d\n", n1);
    errno = 0; int big = snprintf(NULL, 0, "%2147483646d", 1); printf("big1 %d e=%d\n", big, errno);
    errno = 0; big = snprintf(NULL, 0, "%2147483647d", 1); printf("big2 %d e=%d\n", big, errno);
    errno = 0; big = snprintf(NULL, 0, "%2147483647d%d", 1, 1); printf("big3 %d e=%d\n", big, errno);
    errno = 0; big = snprintf(NULL, 0, "%2147483648d", 1); printf("big4 %d e=%d\n", big, errno);
    errno = 0; big = snprintf(NULL, 0, "%.2147483648d", 1); printf("big5 %d e=%d\n", big, errno);
    errno = 0; big = snprintf(NULL, 0, "%*d", INT_MIN, 1); printf("big6 %d e=%d\n", big, errno);
    char b[8] = "XXXXXXX";
    int r = snprintf(b, 4, "%s", "abcdef"); printf("trunc %d [%s] %d\n", r, b, b[4]);
    r = snprintf(b, 0, "%s", "zz"); printf("zero %d [%s]\n", r, b);
    r = snprintf(b, 1, "%s", "zz"); printf("one %d [%s] %d\n", r, b, b[1]);
    r = sprintf(b, "%c%c", 'a', 0); printf("nul %d %d %d\n", r, b[0], b[1]);
    printf("BUFSIZ=%d EOF=%d FOPEN_MAX=%d FILENAME_MAX=%d L_tmpnam=%d TMP_MAX=%d\n", BUFSIZ, EOF, FOPEN_MAX, FILENAME_MAX, L_tmpnam, TMP_MAX);
    printf("SEEK %d %d %d IOFBF %d IOLBF %d IONBF %d\n", SEEK_SET, SEEK_CUR, SEEK_END, _IOFBF, _IOLBF, _IONBF);
    return 0;
}
