/* _exit, POSIX.1-2024 XSH _exit. */
#include <unistd.h>
#include "t.h"

int main(void) {
    t_str("before _exit\n");
    _exit(3);
    t_str("after _exit\n");
    return 1;
}
