/* Direct input/output, C17 7.21.8: fread and fwrite count whole
 * elements; a short read at end of file sets the indicator; a size or a
 * count of zero transfers nothing. Large transfers round-trip through the
 * buffer and past it. Reports through write(2) (t.h); the files are
 * compared by the harness. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "t.h"

static unsigned char big[100000], back[100000];

int main(void) {
    for (unsigned i = 0; i < sizeof big; i++)
        big[i] = (unsigned char)(i * 7 + i / 251);
    FILE *f = fopen("big.bin", "w");
    t_kv("fwrite.1", (long)fwrite(big, 1, 1, f));
    t_kv("fwrite.chunks", (long)fwrite(big + 1, 1000, 50, f));
    t_kv("fwrite.rest", (long)fwrite(big + 50001, 1, sizeof big - 50001, f));
    t_kv("fwrite.size0", (long)fwrite(big, 0, 10, f));
    t_kv("fwrite.n0", (long)fwrite(big, 10, 0, f));
    t_kv("fclose", fclose(f));

    f = fopen("big.bin", "r");
    t_kv("fread.size0", (long)fread(back, 0, 5, f));
    t_kv("fread.n0", (long)fread(back, 5, 0, f));
    t_kv("fread.7", (long)fread(back, 7, 1, f));
    t_kv("fread.bulk", (long)fread(back + 7, 4096, 20, f));
    size_t got = 7 + 4096 * 20;
    t_kv("fread.short_elements", (long)fread(back + got, 1000, 100, f));
    t_kv("feof", feof(f) != 0);
    t_kv("fread.after_eof", (long)fread(back, 1, 10, f));
    fclose(f);
    t_kv("roundtrip.digest_eq", t_digest(back, 7 + 4096 * 20 + 1000 * 17) == t_digest(big, 7 + 4096 * 20 + 1000 * 17));
    t_kv("roundtrip.digest", (long)t_digest(big, sizeof big));

    f = fopen("big.bin", "r");
    memset(back, 0, sizeof back);
    t_kv("fread.all_bytes", (long)fread(back, 1, sizeof back + 10, f));
    t_kv("fread.all_eq", memcmp(back, big, sizeof big) == 0);
    fclose(f);

    f = fopen("w.bin", "w+");
    fwrite("0123456789", 1, 10, f);
    fflush(f);
    lseek(fileno(f), 3, SEEK_SET);
    char b[8];
    t_kv("w+.fread", (long)fread(b, 2, 3, f));
    b[6] = 0;
    t_str(b);
    t_str("\n");
    t_kv("w+.fread_short", (long)fread(b, 2, 3, f));
    t_kv("w+.feof", feof(f) != 0);
    fclose(f);

    f = fopen("big.bin", "r");
    int c = fgetc(f);
    t_kv("mixed.fgetc", c);
    t_kv("mixed.fread", (long)fread(back, 1, 5000, f));
    t_kv("mixed.eq", memcmp(back, big + 1, 5000) == 0);
    t_kv("mixed.fgetc2", fgetc(f) == big[5001]);
    fclose(f);
    return 0;
}
