/* <errno.h>, C17 7.5: errno is a modifiable lvalue of type int, zero
 * at program startup, never set to zero by a library function. */
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "t.h"

int main(void) {
    t_kv("errno.startup", errno);
    t_kv("errno.same_object", &errno == &errno);
    errno = 1234;
    t_kv("errno.assigned", errno);
    errno += 1;
    t_kv("errno.compound", errno);

    errno = 0;
    long r = write(-1, "x", 1);
    t_kv("errno.write_badf.ret", r);
    t_kv("errno.write_badf_is_EBADF", errno == EBADF);

    /* A successful call leaves errno alone. */
    errno = 77;
    write(1, "", 0);
    t_kv("errno.kept_on_success", errno);

    errno = 0;
    void *p = malloc((size_t)-1 / 4);
    t_kv("errno.malloc_null", p == NULL);
    t_kv("errno.malloc_is_ENOMEM", errno == ENOMEM);
    t_kv("errno.values", EBADF * 10000 + ENOMEM * 100 + EINVAL);
    return 0;
}
