/* selftest plant: an own case whose check fails. */
#include <unistd.h>
int main(void) {
    write(1, "FAIL breaks\n", 12);
    return 1;
}
