/* selftest plant: only stdout differs between the columns. */
#include <stdio.h>
int main(void) {
#ifdef __GLIBC__
    puts("glibc column");
#else
    puts("muzzle column");
#endif
    return 0;
}
