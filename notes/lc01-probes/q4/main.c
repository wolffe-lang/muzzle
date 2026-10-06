#include <stdio.h>
int *q_errno_loc(void);
int q_errno_get(void);
int main(void) {
    printf("q_errno_loc=%p q_errno_get=%d\n", (void *)q_errno_loc(), q_errno_get());
    return 0;
}
