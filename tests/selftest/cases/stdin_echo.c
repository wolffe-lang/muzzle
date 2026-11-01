/* selftest control: stdin is stdin_echo.in, echoed byte for byte. */
#include <stdio.h>
int main(void) {
    int c;
    while ((c = getchar()) != EOF)
        putchar(c);
    return 0;
}
