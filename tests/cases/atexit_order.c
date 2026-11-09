/* C17 7.22.4.2 atexit, 7.22.4.4 exit: the handlers run in the reverse of
 * their registration, a function registered twice runs twice, and only
 * then are the streams flushed (p4), so a handler's buffered output
 * reaches the file after main's. stdout is a file here: fully buffered,
 * so every line arrives at exit. */
#include <stdio.h>
#include <stdlib.h>

static void h1(void) { printf("h1\n"); }
static void h2(void) { printf("h2\n"); }
static void h3(void) { printf("h3 (registered first)\n"); }

int main(void) {
    printf("atexit=%d\n", atexit(h3));
    printf("atexit=%d\n", atexit(h2));
    printf("atexit=%d\n", atexit(h1));
    printf("atexit=%d\n", atexit(h2));
    printf("main\n");
    exit(3);
}
