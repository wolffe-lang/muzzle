/* Returning from main is exit with that value, C17 5.1.2.2.3. */
#include "t.h"

int main(void) {
    t_str("returning 7\n");
    return 7;
}
