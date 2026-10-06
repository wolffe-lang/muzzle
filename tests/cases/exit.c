/* exit, C17 7.22.4.4: the status's low eight bits reach the parent
 * (POSIX XSH exit); nothing after the call runs. */
#include <stdlib.h>
#include "t.h"

int main(void) {
    t_str("before exit\n");
    exit(256 + 42);
    t_str("after exit\n");
    return 1;
}
