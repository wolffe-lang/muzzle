/* The environment: C17 7.22.4.6 getenv; POSIX.1-2024 setenv, unsetenv
 * and environ (XBD 8.1: the program may read environ, and may assign it
 * a list of its own). The case prints only what it set itself, the
 * count of entries and positions relative to it, so the host's own
 * variables never reach the output. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;

static int count(void) {
    int n = 0;
    while (environ[n] != NULL)
        n++;
    return n;
}

static int where(const char *entry) {
    for (int i = 0; environ[i] != NULL; i++)
        if (strcmp(environ[i], entry) == 0)
            return i;
    return -1;
}

static void show(const char *name) {
    const char *v = getenv(name);
    printf("getenv(%s)=%s%s%s\n", name, v ? "[" : "", v ? v : "(null)", v ? "]" : "");
}

int main(void) {
    int base = count();
    const char *path = getenv("PATH");
    printf("PATH set=%d\n", path != NULL);
    show("LC03_A");
    errno = 0;
    printf("setenv new r=%d errno=%d count=+%d\n", setenv("LC03_A", "one", 0), errno, count() - base);
    show("LC03_A");
    printf("LC03_A at end-1=%d\n", where("LC03_A=one") == count() - 1);
    printf("setenv keep r=%d\n", setenv("LC03_A", "two", 0));
    show("LC03_A");
    printf("setenv over r=%d count=+%d\n", setenv("LC03_A", "two", 1), count() - base);
    show("LC03_A");
    printf("LC03_A at end-1=%d\n", where("LC03_A=two") == count() - 1);
    printf("setenv eq r=%d\n", setenv("LC03_B", "x=y=", 1));
    show("LC03_B");
    printf("setenv empty value r=%d\n", setenv("LC03_C", "", 1));
    show("LC03_C");
    show("LC03");
    show("LC03_");
    show("");

    static const char *const bad[] = {"", "A=B", "=", "LC03_A="};
    for (size_t i = 0; i < sizeof bad / sizeof *bad; i++) {
        errno = 0;
        int r = setenv(bad[i], "v", 1);
        int e1 = errno;
        errno = 0;
        int u = unsetenv(bad[i]);
        printf("bad [%s] setenv=%d/%d unsetenv=%d/%d\n", bad[i], r, e1, u, errno);
    }
    const char *volatile nul = NULL; /* past the headers' nonnull */
    errno = 0;
    int rn = setenv(nul, "v", 1);
    printf("setenv(NULL) r=%d errno=%d\n", rn, errno);
    errno = 0;
    rn = unsetenv(nul);
    printf("unsetenv(NULL) r=%d errno=%d\n", rn, errno);

    errno = 0;
    printf("unsetenv r=%d errno=%d count=+%d\n", unsetenv("LC03_A"), errno, count() - base);
    show("LC03_A");
    printf("unsetenv absent r=%d count=+%d\n", unsetenv("LC03_A"), count() - base);
    show("LC03_B");

    /* Many: 200 set, every third unset, the rest checked in order. */
    char name[32], val[32], want[64];
    for (int i = 0; i < 200; i++) {
        snprintf(name, sizeof name, "LC03_V%d", i);
        snprintf(val, sizeof val, "%d", i * 7);
        setenv(name, val, 1);
    }
    printf("many count=+%d\n", count() - base);
    for (int i = 0; i < 200; i += 3) {
        snprintf(name, sizeof name, "LC03_V%d", i);
        unsetenv(name);
    }
    int ok = 1, order = 1, last = -1;
    for (int i = 0; i < 200; i++) {
        snprintf(name, sizeof name, "LC03_V%d", i);
        const char *v = getenv(name);
        if (i % 3 == 0) {
            ok &= v == NULL;
            continue;
        }
        snprintf(want, sizeof want, "%d", i * 7);
        ok &= v != NULL && strcmp(v, want) == 0;
        snprintf(want, sizeof want, "LC03_V%d=%d", i, i * 7);
        int at = where(want);
        order &= at > last;
        last = at;
    }
    printf("many ok=%d order=%d count=+%d\n", ok, order, count() - base);
    int dup = 0;
    for (int i = 0; environ[i] != NULL; i++)
        for (int j = i + 1; environ[j] != NULL; j++)
            dup += strcmp(environ[i], environ[j]) == 0;
    printf("duplicates=%d\n", dup);

    /* A list the program assigns to environ is what getenv reads. */
    char **saved = environ;
    static char e1[] = "MINE=1", e2[] = "OTHER=two", e3[] = "MINE=shadow";
    static char *mine[] = {e1, e2, e3, NULL};
    environ = mine;
    show("MINE");
    show("OTHER");
    show("PATH");
    environ = saved;
    show("LC03_V1");
    printf("PATH again=%d\n", getenv("PATH") == path || (getenv("PATH") && path && strcmp(getenv("PATH"), path) == 0));
    return 0;
}
