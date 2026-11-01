/* muzzle's own: fflush of an input stream with read-ahead and a pushed-back
 * byte both pending. POSIX XSH fflush: "the file offset of the underlying
 * open file description shall be set to the file position of the stream,
 * and any characters pushed back onto the stream by ungetc() ... shall be
 * discarded (without further changing the file offset)". Reading "abcdef"
 * up to 'c' leaves the position at 3; ungetc makes it 2; fflush sets the
 * offset to 2 and drops the pushback, so the next byte is 'c' again.
 * glibc instead moves the offset back over the pushback alone (to 5) and
 * keeps its read-ahead, so the next byte is 'd' (docs/LEDGER.md): the
 * standard is the oracle here, and this case checks muzzle against it. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int failed;

static void check(const char *name, int cond) {
    char line[96];
    int n = snprintf(line, sizeof line, "%s %s\n", cond ? "ok" : "FAIL", name);
    write(1, line, (size_t)n);
    if (!cond)
        failed = 1;
}

int main(void) {
    FILE *f = fopen("p.txt", "w");
    fputs("abcdef", f);
    fclose(f);
    f = fopen("p.txt", "r");
    check("read_a", fgetc(f) == 'a');
    check("read_b", fgetc(f) == 'b');
    check("read_c", fgetc(f) == 'c');
    check("unget", ungetc('W', f) == 'W');
    check("fflush", fflush(f) == 0);
    check("offset_is_position", lseek(fileno(f), 0, SEEK_CUR) == 2);
    check("next_is_c", fgetc(f) == 'c');
    check("then_d", fgetc(f) == 'd');
    fclose(f);
    return failed;
}
