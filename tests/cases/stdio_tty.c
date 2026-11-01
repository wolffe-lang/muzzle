/* stdout on a terminal is line buffered (C17 7.21.3p7: fully buffered
 * only when it "can be determined not to refer to an interactive
 * device"): a newline sends the line; a partial line waits, until a read
 * from stdin needs the host (7.21.3p3) or exit. The harness runs this
 * case with stdout on a pseudo-terminal; direct write(2)s mark when the
 * stream's bytes arrived. stdin is stdio_tty.in. */
// difftest: tty
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void mark(const char *m) { write(1, m, strlen(m)); }

int main(void) {
    char t[32];
    int n = snprintf(t, sizeof t, "<tty=%d>", isatty(1));
    write(1, t, (size_t)n);
    printf("a");
    mark("[1]");
    printf("b\nc");
    mark("[2]");
    fputs("d", stdout);
    mark("[3]");
    putchar('e');
    mark("[4]");
    fputs("f\n", stdout);
    mark("[5]");
    fwrite("g\nh", 1, 3, stdout);
    mark("[6]");
    putchar('\n');
    mark("[7]");
    puts("i");
    mark("[8]");
    printf("j");
    getchar();
    mark("[9]");
    fprintf(stderr, "e1");
    mark("[10]");
    printf("k");
    return 0;
}
