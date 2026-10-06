#include <stdio.h>
long q9_noread(void);
long q9_reread(void);
int main(void) {
    printf("noread=%ld (want 99)\n", q9_noread());
    printf("reread=%ld (want 99)\n", q9_reread());
    return 0;
}
