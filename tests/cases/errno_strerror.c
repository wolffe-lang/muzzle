/* <errno.h> and the error texts: C17 7.5 (the E* macros; POSIX.1-2024
 * XBD errno.h names the rest), 7.24.6.2 strerror, 7.21.10.4 perror.
 * Every number Linux defines is printed by name, so the two headers must
 * agree on each one, and strerror's text for every number from -2 to 140
 * (and a few far ones) must equal glibc's in the C locale, including
 * "Unknown error N" for the numbers no error has. strerror and perror
 * leave errno alone. perror writes to stderr, which the harness compares. */
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define E(n) {#n, n}
static const struct { const char *name; int value; } names[] = {
    E(EPERM),
    E(ENOENT),
    E(ESRCH),
    E(EINTR),
    E(EIO),
    E(ENXIO),
    E(E2BIG),
    E(ENOEXEC),
    E(EBADF),
    E(ECHILD),
    E(EAGAIN),
    E(ENOMEM),
    E(EACCES),
    E(EFAULT),
    E(ENOTBLK),
    E(EBUSY),
    E(EEXIST),
    E(EXDEV),
    E(ENODEV),
    E(ENOTDIR),
    E(EISDIR),
    E(EINVAL),
    E(ENFILE),
    E(EMFILE),
    E(ENOTTY),
    E(ETXTBSY),
    E(EFBIG),
    E(ENOSPC),
    E(ESPIPE),
    E(EROFS),
    E(EMLINK),
    E(EPIPE),
    E(EDOM),
    E(ERANGE),
    E(EDEADLK),
    E(ENAMETOOLONG),
    E(ENOLCK),
    E(ENOSYS),
    E(ENOTEMPTY),
    E(ELOOP),
    E(EWOULDBLOCK),
    E(ENOMSG),
    E(EIDRM),
    E(ECHRNG),
    E(EL2NSYNC),
    E(EL3HLT),
    E(EL3RST),
    E(ELNRNG),
    E(EUNATCH),
    E(ENOCSI),
    E(EL2HLT),
    E(EBADE),
    E(EBADR),
    E(EXFULL),
    E(ENOANO),
    E(EBADRQC),
    E(EBADSLT),
    E(EDEADLOCK),
    E(EBFONT),
    E(ENOSTR),
    E(ENODATA),
    E(ETIME),
    E(ENOSR),
    E(ENONET),
    E(ENOPKG),
    E(EREMOTE),
    E(ENOLINK),
    E(EADV),
    E(ESRMNT),
    E(ECOMM),
    E(EPROTO),
    E(EMULTIHOP),
    E(EDOTDOT),
    E(EBADMSG),
    E(EOVERFLOW),
    E(ENOTUNIQ),
    E(EBADFD),
    E(EREMCHG),
    E(ELIBACC),
    E(ELIBBAD),
    E(ELIBSCN),
    E(ELIBMAX),
    E(ELIBEXEC),
    E(EILSEQ),
    E(ERESTART),
    E(ESTRPIPE),
    E(EUSERS),
    E(ENOTSOCK),
    E(EDESTADDRREQ),
    E(EMSGSIZE),
    E(EPROTOTYPE),
    E(ENOPROTOOPT),
    E(EPROTONOSUPPORT),
    E(ESOCKTNOSUPPORT),
    E(EOPNOTSUPP),
    E(ENOTSUP),
    E(EPFNOSUPPORT),
    E(EAFNOSUPPORT),
    E(EADDRINUSE),
    E(EADDRNOTAVAIL),
    E(ENETDOWN),
    E(ENETUNREACH),
    E(ENETRESET),
    E(ECONNABORTED),
    E(ECONNRESET),
    E(ENOBUFS),
    E(EISCONN),
    E(ENOTCONN),
    E(ESHUTDOWN),
    E(ETOOMANYREFS),
    E(ETIMEDOUT),
    E(ECONNREFUSED),
    E(EHOSTDOWN),
    E(EHOSTUNREACH),
    E(EALREADY),
    E(EINPROGRESS),
    E(ESTALE),
    E(EUCLEAN),
    E(ENOTNAM),
    E(ENAVAIL),
    E(EISNAM),
    E(EREMOTEIO),
    E(EDQUOT),
    E(ENOMEDIUM),
    E(EMEDIUMTYPE),
    E(ECANCELED),
    E(ENOKEY),
    E(EKEYEXPIRED),
    E(EKEYREVOKED),
    E(EKEYREJECTED),
    E(EOWNERDEAD),
    E(ENOTRECOVERABLE),
    E(ERFKILL),
    E(EHWPOISON),
};

int main(void) {
    for (size_t i = 0; i < sizeof names / sizeof *names; i++)
        printf("%s=%d\n", names[i].name, names[i].value);
    printf("count=%zu\n", sizeof names / sizeof *names);
    for (int e = -2; e <= 140; e++) {
        errno = 4242;
        const char *s = strerror(e);
        printf("strerror(%d)=[%s] errno=%d\n", e, s, errno);
    }
    static const int far[] = {1000, 4095, 4096, 65536, INT_MAX, INT_MIN, -4095};
    for (size_t i = 0; i < sizeof far / sizeof *far; i++)
        printf("strerror(%d)=[%s]\n", far[i], strerror(far[i]));
    /* The text does not change between calls. */
    char first[64];
    snprintf(first, sizeof first, "%s", strerror(EACCES));
    printf("again=%d\n", strcmp(first, strerror(EACCES)) == 0);

    /* glibc's first perror on a stderr that has had no output leaves
     * errno EINVAL, whatever it was (probes g2 and g3, docs/LEDGER.md);
     * muzzle leaves errno alone. stderr gets a line first, so the rows
     * below compare what both promise. */
    fputs("perror rows:\n", stderr);
    static const int pe[] = {0, EPERM, ENOENT, EINVAL, ERANGE, EHWPOISON, 41, 134, 1000, -1};
    for (size_t i = 0; i < sizeof pe / sizeof *pe; i++) {
        errno = pe[i];
        perror("lc03");
        printf("perror kept=%d\n", errno == pe[i]);
    }
    errno = EBADF;
    perror("");
    errno = EBADF;
    perror(NULL);
    errno = ENOSPC;
    perror("a: b");
    fprintf(stderr, "end\n");
    return 0;
}
