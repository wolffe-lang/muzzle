/* Returning from main is exit with its value (C17 5.1.2.2.3): the
 * handlers run in reverse. C17 7.22.4.2p3 requires room for at least 32;
 * this registers 40 of one function among 3 others. */
#include <stdio.h>
#include <stdlib.h>

static int left = 40;
static void many(void) { printf("many %d\n", left--); }
static void a(void) { printf("a\n"); }
static void b(void) { printf("b\n"); }
static void c(void) { printf("c\n"); }

int main(void) {
    int ok = atexit(a) == 0;
    for (int i = 0; i < 20; i++)
        ok &= atexit(many) == 0;
    ok &= atexit(b) == 0;
    for (int i = 0; i < 20; i++)
        ok &= atexit(many) == 0;
    ok &= atexit(c) == 0;
    printf("registered=%d\n", ok);
    return 7;
}
