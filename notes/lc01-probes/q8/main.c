#include <stdio.h>
long q8_reread(void);
long q8_noread(void);
void q8_set(long);
long q8_get(void);
long q8_same(long);
long q8_call_arg(long);
int main(void) {
    printf("reread=%ld (want 99)\nnoread=%ld (want 99)\n", q8_reread(), q8_noread());
    q8_set(5);
    printf("set(5) then get=%ld (want 5)\n", q8_get());
    printf("same(6)=%ld (want 6)\n", q8_same(6));
    printf("get after same(6)=%ld (want 6)\n", q8_get());
    printf("call_arg(8)=%ld (want 8)\n", q8_call_arg(8));
    printf("get after call_arg(8)=%ld (want 8)\n", q8_get());
    return 0;
}
