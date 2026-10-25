/* Fifty defined C17 printf-subset cases. No libc source or headers were
 * read: stdio.h is consumed by clang, with muzzle's own synopsis in the
 * freestanding column. Each primary call checks an independently spelled
 * expected byte count; all output (including embedded NULs) is compared
 * against black-box glibc by tools/difftest. Use printf for metadata too,
 * so glibc's buffering cannot reorder it relative to a write helper. */
#include <stdio.h>

#define CASE(expected, ...) do { \
    int n = printf(__VA_ARGS__); \
    ++cases; \
    if (n != (int)(sizeof(expected) - 1)) ++failures; \
    if (printf("|case=%d ret=%d\n", cases, n) < 0) ++failures; \
} while (0)

int main(void) {
    int cases = 0, failures = 0;
    /* Literal and percent paths, including an empty result. */
    CASE("", "%s", "");                                      /* 01 */
    CASE("plain text", "plain text");                         /* 02 */
    CASE("line one\nline two\n", "line one\nline two\n");       /* 03 */
    CASE("%", "%%");                                         /* 04 */
    CASE("%%%", "%%%%%%");                                   /* 05 */
    CASE("before%after", "before%%after");                    /* 06 */
    CASE("0", "%d", 0);                                      /* 07 */
    CASE("1", "%d", 1);                                      /* 08 */
    CASE("-1", "%d", -1);                                    /* 09 */
    CASE("9 10 99 100", "%d %d %d %d", 9, 10, 99, 100);       /* 10 */
    CASE("-9 -10 -99 -100", "%d %d %d %d", -9, -10, -99, -100); /* 11 */
    CASE("2147483647", "%d", 2147483647);                     /* 12 */
    CASE("-2147483648", "%d", (-2147483647 - 1));              /* 13 */
    CASE("-128", "%d", (signed char)-128);                    /* 14 */
    CASE("127", "%d", (signed char)127);                      /* 15 */
    CASE("255", "%d", (unsigned char)255);                    /* 16 */
    CASE("-32768", "%d", (short)-32768);                      /* 17 */
    CASE("65535", "%d", (unsigned short)65535);               /* 18 */
    CASE("0 1", "%d %d", (_Bool)0, (_Bool)1);                 /* 19 */
    CASE("1234567890", "%d", 1234567890);                     /* 20 */
    CASE("0", "%x", 0u);                                     /* 21 */
    CASE("1", "%x", 1u);                                     /* 22 */
    CASE("9 a f 10", "%x %x %x %x", 9u, 10u, 15u, 16u);     /* 23 */
    CASE("ff 100", "%x %x", 255u, 256u);                      /* 24 */
    CASE("7fffffff", "%x", 2147483647u);                      /* 25 */
    CASE("80000000", "%x", 2147483648u);                      /* 26 */
    CASE("ffffffff", "%x", 4294967295u);                      /* 27 */
    CASE("deadbeef", "%x", 3735928559u);                      /* 28 */
    CASE("a5a5a5a5", "%x", 2779096485u);                      /* 29 */
    CASE("A", "%c", 'A');                                    /* 30 */
    CASE("\0", "%c", 0);                                    /* 31 */
    CASE("\xff", "%c", 255);                                /* 32 */
    CASE("A", "%c", 321);                                    /* 33 */
    CASE("\xff", "%c", -1);                                 /* 34 */
    CASE("\n\t ", "%c%c%c", '\n', '\t', ' ');                /* 35 */
    CASE("wolf", "%s", "wolf");                              /* 36 */
    CASE("[a][b][]", "[%s][%s][%s]", "a", "b", "");          /* 37 */
    CASE("literal %d %s %x", "%s", "literal %d %s %x");       /* 38 */
    CASE("prefix", "%s", "prefix\0not printed");              /* 39 */
    CASE("\xc3\xa9", "%s", "\xc3\xa9");                     /* 40 */
    CASE("a\0b", "%s%c%s", "a", 0, "b");                     /* 41 */
    CASE("-42/wolf/Z/feed/%", "%d/%s/%c/%x/%%", -42, "wolf", 'Z', 65261u); /* 42 */
    CASE("1 2 3 4 5", "%d %d %d %d %d", 1, 2, 3, 4, 5);     /* 43: all remaining GP */
    CASE("1 2 3 4 5 6", "%d %d %d %d %d %d", 1, 2, 3, 4, 5, 6); /* 44: first stack */
    CASE("1 2 3 4 5 6 7 8 9 10 11 12", "%d %d %d %d %d %d %d %d %d %d %d %d",
         1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);             /* 45 */
    CASE("a/b/c/d/e/f/g/h", "%s/%s/%s/%s/%s/%s/%s/%s",
         "a", "b", "c", "d", "e", "f", "g", "h");         /* 46 */
    CASE("0/1/a/f/10/ff/ffff/ffffffff", "%x/%x/%x/%x/%x/%x/%x/%x",
         0u, 1u, 10u, 15u, 16u, 255u, 65535u, 4294967295u);  /* 47 */
    CASE("ABCDEFGH", "%c%c%c%c%c%c%c%c", 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'); /* 48 */
    CASE("-1:a:B:ff:5:tail:C:80000000:-2147483648", "%d:%s:%c:%x:%d:%s:%c:%x:%d",
         -1, "a", 'B', 255u, 5, "tail", 'C', 2147483648u, (-2147483647 - 1)); /* 49 */
    CASE("0123456789012345678901234567890123456789012345678901234567890123456789%end",
         "%s%%%s", "0123456789012345678901234567890123456789012345678901234567890123456789", "end"); /* 50 */
    printf("cases=%d failures=%d\n", cases, failures);
    return cases != 50 || failures != 0;
}
