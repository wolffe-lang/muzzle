/* Muzzle-only subset/error checks, not glibc comparisons of unsupported
 * syntax or buffered stream timing. syscall close is a test fixture only. */
#include <errno.h>
#include <stdio.h>

static long close_stdout(void) {
    long result;
    __asm__ volatile ("syscall" : "=a"(result) : "a"(3L), "D"(1L)
                      : "rcx", "r11", "memory");
    return result;
}

int main(void) {
    const char *unsupported = "%q";
    const char *trailing = "%";
    errno = 0;
    if (printf(unsupported) != -1 || errno != EINVAL) return 1;
    errno = 0;
    if (printf(trailing) != -1 || errno != EINVAL) return 2;
    if (printf("ok\n") != 3) return 3;
    if (close_stdout() != 0) return 4;
    errno = 0;
    if (printf("%d", 42) != -1 || errno != EBADF) return 5;
    /* An empty format performs no write, even on a closed descriptor. */
    if (printf("%s", "") != 0) return 6;
    return 0;
}
