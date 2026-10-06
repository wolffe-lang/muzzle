#include <stdio.h>
long q7(int);
int main(void) {
    const char *n[] = {"if !f(x) {return}", "let g = f(x); if !g", "if f(x) == false", "if !plain(x), no unsafe/var"};
    for (int k = 0; k < 4; k++) printf("%-30s -> %ld (want 7)\n", n[k], q7(k));
    return 0;
}
