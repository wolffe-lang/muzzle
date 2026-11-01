/* ptyrun: run a program with its standard output on a pseudo-terminal,
 * copy what it writes there to our standard output, byte for byte, and
 * exit with its status (128+n for a signal, as a shell reports it).
 * Standard input and standard error are inherited unchanged. The
 * terminal's output processing (OPOST) is off, so the bytes arrive as
 * written: no newline translation.
 *
 * A harness tool, built against the host C library (never muzzle).
 * Written from POSIX.1-2024 XSH posix_openpt, grantpt, unlockpt,
 * ptsname, tcgetattr, tcsetattr, fork, dup2, execv, waitpid, and the
 * termios(3) and pty(7) man pages.
 *   ptyrun PROGRAM [ARG...] */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: ptyrun PROGRAM [ARG...]\n");
        return 2;
    }
    int m = posix_openpt(O_RDWR | O_NOCTTY);
    if (m < 0 || grantpt(m) != 0 || unlockpt(m) != 0) {
        perror("ptyrun: pty");
        return 2;
    }
    char *name = ptsname(m);
    int s = name ? open(name, O_RDWR | O_NOCTTY) : -1;
    if (s < 0) {
        perror("ptyrun: slave");
        return 2;
    }
    struct termios t;
    if (tcgetattr(s, &t) != 0) {
        perror("ptyrun: tcgetattr");
        return 2;
    }
    t.c_oflag &= ~(tcflag_t)OPOST;
    t.c_lflag &= ~(tcflag_t)ECHO;
    if (tcsetattr(s, TCSANOW, &t) != 0) {
        perror("ptyrun: tcsetattr");
        return 2;
    }
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) {
        perror("ptyrun: fork");
        return 2;
    }
    if (pid == 0) {
        close(m);
        if (dup2(s, 1) < 0)
            _exit(126);
        if (s != 1)
            close(s);
        execv(argv[1], argv + 1);
        _exit(127);
    }
    close(s);
    char buf[4096];
    for (;;) {
        ssize_t n = read(m, buf, sizeof buf);
        if (n > 0) {
            ssize_t off = 0;
            while (off < n) {
                ssize_t w = write(1, buf + off, (size_t)(n - off));
                if (w < 0) {
                    if (errno == EINTR)
                        continue;
                    perror("ptyrun: write");
                    return 2;
                }
                off += w;
            }
            continue;
        }
        if (n < 0 && errno == EINTR)
            continue;
        break; /* EIO once every slave descriptor is closed, or EOF */
    }
    int st;
    while (waitpid(pid, &st, 0) < 0) {
        if (errno != EINTR) {
            perror("ptyrun: waitpid");
            return 2;
        }
    }
    if (WIFEXITED(st))
        return WEXITSTATUS(st);
    if (WIFSIGNALED(st))
        return 128 + WTERMSIG(st);
    return 2;
}
