/* selftest plant: only the written file differs between the columns. */
#include <stdio.h>
int main(void) {
    FILE *f = fopen("out.txt", "w");
#ifdef __GLIBC__
    fputs("glibc column\n", f);
#else
    fputs("muzzle column\n", f);
#endif
    fclose(f);
    puts("same stdout");
    return 0;
}
