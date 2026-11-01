/* selftest control: an own case whose one check holds. */
#include <unistd.h>
int main(void) {
    write(1, "ok holds\n", 9);
    return 0;
}
