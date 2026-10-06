/* write, POSIX.1-2024 XSH write: returns the bytes written, or -1 with
 * errno set. */
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "t.h"

int main(void) {
    t_kv("write.zero", write(1, "abc", 0));
    long n = write(STDOUT_FILENO, "howl\n", 5);
    t_kv("write.n", n);
    n = write(1, "partial-NOT-THIS", 8);
    t_str("\n");
    t_kv("write.partial", n);
    errno = 0;
    n = write(1, (const void *)8, 4);
    t_kv("write.efault.ret", n);
    t_kv("write.efault_is_EFAULT", errno == EFAULT);
    errno = 0;
    n = write(99, "x", 1);
    t_kv("write.ebadf.ret", n);
    t_kv("write.ebadf_is_EBADF", errno == EBADF);
    return 0;
}
