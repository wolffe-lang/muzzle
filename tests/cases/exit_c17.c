/* _Exit, C17 7.22.4.5. */
#include <stdlib.h>
#include "t.h"

int main(void) {
    t_str("before _Exit\n");
    _Exit(250);
    t_str("after _Exit\n");
    return 1;
}
