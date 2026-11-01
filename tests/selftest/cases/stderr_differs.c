/* selftest plant: only stderr differs between the columns. */
#include <stdio.h>
int main(void) {
#ifdef __GLIBC__
    fputs("glibc column\n", stderr);
#else
    fputs("muzzle column\n", stderr);
#endif
    puts("same stdout");
    return 0;
}
