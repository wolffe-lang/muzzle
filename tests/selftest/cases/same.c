/* selftest control: the same in both columns. */
#include <stdio.h>
int main(void) {
    FILE *f = fopen("same.txt", "w");
    fputs("same bytes\n", f);
    fclose(f);
    fputs("same stderr\n", stderr);
    puts("same stdout");
    return 0;
}
