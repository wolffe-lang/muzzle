/* selftest plant: only the exit status differs between the columns. */
#include <stdio.h>
int main(void) {
    puts("same stdout");
#ifdef __GLIBC__
    return 0;
#else
    return 3;
#endif
}
