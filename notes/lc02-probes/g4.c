/* g4: glibc's stdout on a terminal (run under ptyrun), as a black box:
 * which bytes reach the terminal before a direct write(2). */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void mark(const char *m) { write(1, m, strlen(m)); }

int main(void) {
    char t[24];
    int n = snprintf(t, sizeof t, "<tty=%d>", isatty(1));
    write(1, t, (size_t)n);
    printf("a"); mark("[1]");
    printf("b\nc"); mark("[2]");
    fputs("d", stdout); mark("[3]");
    putchar('e'); mark("[4]");
    fputs("f\n", stdout); mark("[5]");
    fwrite("g\nh", 1, 3, stdout); mark("[6]");
    putchar('\n'); mark("[7]");
    printf("i"); getchar(); mark("[8]");
    fprintf(stderr, "x"); mark("[9]");
    printf("j");
    return 0;
}
