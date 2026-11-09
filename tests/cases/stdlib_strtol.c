/* <stdlib.h> integer conversions: C17 7.22.1.4 strtol, strtoll, strtoul,
 * strtoull; 7.22.1.2 atoi, atol, atoll. Each input runs through every
 * function and base; a line shows the value, how far the end pointer
 * moved (or `same` when it was left alone), and errno. */
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static const char *in[] = {
    "0", "1", "-1", "+1", "  42", "\t\n\v\f\r 42x", "-0", "+-1", "--1", "", " ", "  +", "  -",
    "0x", "0X", "0x1f", "0X1F", "0xg", "-0x10", "0x-1", "0b101", "017", "08", "019", "0", "00", "0x0",
    "z", "Z", "zz", "1z", "  -0x1Z", "123abc", "9", "a", "777", "1_000",
    "2147483647", "2147483648", "-2147483648", "-2147483649", "4294967295", "4294967296",
    "9223372036854775807", "9223372036854775808", "-9223372036854775808", "-9223372036854775809",
    "18446744073709551615", "18446744073709551616", "-18446744073709551615", "-18446744073709551616",
    "99999999999999999999999999", "-99999999999999999999999999", "0x7fffffffffffffff", "0x8000000000000000",
    "0xffffffffffffffff", "0x10000000000000000", "-0x8000000000000000", "-0x8000000000000001",
    "1111111111111111111111111111111111111111111111111111111111111111",
    "11111111111111111111111111111111111111111111111111111111111111111",
    "3w5e11264sgsf", "3w5e11264sgsg", "zzzzzzzzzzzzz", "000000000000000000000000000000001",
};
static const int bases[] = {0, 2, 8, 10, 16, 36, 1, 37, -1, 3, 7, 11, 35};

#define END(e, s) ((e) == sentinel ? -1L : (long)((e) - (s)))

int main(void) {
    char sentinel[] = "x";
    for (size_t i = 0; i < sizeof in / sizeof *in; i++) {
        const char *s = in[i];
        for (size_t j = 0; j < sizeof bases / sizeof *bases; j++) {
            int b = bases[j];
            char *e;
            e = sentinel; errno = 0;
            long l = strtol(s, &e, b);
            printf("[%s] b=%d l=%ld/%ld/%d", s, b, l, END(e, s), errno);
            e = sentinel; errno = 0;
            long long ll = strtoll(s, &e, b);
            printf(" ll=%lld/%ld/%d", ll, END(e, s), errno);
            e = sentinel; errno = 0;
            unsigned long ul = strtoul(s, &e, b);
            printf(" ul=%lu/%ld/%d", ul, END(e, s), errno);
            e = sentinel; errno = 0;
            unsigned long long ull = strtoull(s, &e, b);
            printf(" ull=%llu/%ld/%d\n", ull, END(e, s), errno);
        }
        /* A null end pointer is allowed. */
        errno = 0;
        long n = strtol(s, NULL, 10);
        printf("[%s] null-end=%ld/%d", s, n, errno);
        /* atoi and friends: C17 7.22.1.2 is strtol(s, NULL, 10) except on
         * error, where the behaviour is undefined, so only values that fit
         * an int are given to atoi. */
        long chk = strtol(s, NULL, 10);
        if (chk >= INT_MIN && chk <= INT_MAX)
            printf(" atoi=%d", atoi(s));
        printf(" atol=%ld atoll=%lld\n", atol(s), atoll(s));
    }
    return 0;
}
