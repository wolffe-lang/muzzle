/* ungetc, C17 7.21.7.10: one byte pushed back is guaranteed, read next,
 * as unsigned char; it clears the end-of-file indicator; EOF is never
 * pushed. Two pushed back come out last in first out (glibc takes
 * more than one; docs/LEDGER.md). fflush gives read-ahead back to the
 * file and drops pushed-back bytes, the offset moved back over them (POSIX
 * XSH fflush), here with the buffer empty. With read-ahead and pushback
 * both pending glibc departs from POSIX (docs/LEDGER.md), so that shape
 * is muzzle's own case (tests/own/fflush_pushback.c). Reports through
 * write(2) (t.h). */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include "t.h"

int main(void) {
    FILE *f = fopen("u.txt", "w");
    fputs("abcdef", f);
    fclose(f);

    f = fopen("u.txt", "r");
    t_kv("unget.fresh", ungetc('Q', f));
    t_kv("read.fresh", fgetc(f));
    t_kv("read.a", fgetc(f));
    t_kv("unget.Z", ungetc('Z', f));
    t_kv("read.Z", fgetc(f));
    t_kv("read.b", fgetc(f));
    t_kv("unget.Y", ungetc('Y', f));
    t_kv("unget.X", ungetc('X', f));
    t_kv("read.X", fgetc(f));
    t_kv("read.Y", fgetc(f));
    t_kv("read.c", fgetc(f));
    t_kv("unget.EOF", ungetc(EOF, f));
    t_kv("unget.0x1ff", ungetc(0x1ff, f));
    t_kv("read.0xff", fgetc(f));
    char b[8];
    t_kv("unget.for_fread", ungetc('R', f));
    t_kv("fread.n", (long)fread(b, 1, 3, f));
    b[3] = 0;
    t_str(b);
    t_str("\n");
    while (fgetc(f) != EOF) {
    }
    t_kv("eof.set", feof(f) != 0);
    t_kv("unget.at_eof", ungetc('k', f));
    t_kv("eof.cleared", feof(f));
    t_kv("read.k", fgetc(f));
    t_kv("read.eof_again", fgetc(f));
    fclose(f);

    f = fopen("u.txt", "r");
    fgetc(f);
    fgetc(f);
    t_kv("offset.after_two_reads", lseek(fileno(f), 0, SEEK_CUR));
    t_kv("fflush.r", fflush(f));
    t_kv("offset.after_fflush", lseek(fileno(f), 0, SEEK_CUR));
    t_kv("fflush.empty", fflush(f));
    ungetc('W', f);
    t_kv("fflush.with_pushback", fflush(f));
    t_kv("offset.with_pushback", lseek(fileno(f), 0, SEEK_CUR));
    t_kv("read.after_pushback_flush", fgetc(f));
    fclose(f);
    return 0;
}
