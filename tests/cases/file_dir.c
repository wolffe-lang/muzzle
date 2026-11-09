/* Directories: POSIX.1-2024 XSH opendir, readdir, closedir, getcwd and
 * chdir. readdir's order is the file system's, so names are collected
 * and sorted before they print; d_type is printed as the file system
 * reports it. The working directory's absolute path differs between the
 * columns, so only its relation to the starting one is printed. */
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHOW(what, expr) do { errno = 0; long r_ = (long)(expr); printf("%s r=%ld errno=%d\n", what, r_, errno); } while (0)

static int by_name(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }

static char *names[1200];
static int types[1200];

static void list(const char *path, int quiet) {
    errno = 0;
    DIR *d = opendir(path);
    if (d == NULL) {
        printf("opendir(%s) null errno=%d\n", path, errno);
        return;
    }
    int n = 0;
    struct dirent *e;
    errno = 0;
    while ((e = readdir(d)) != NULL) {
        size_t len = strlen(e->d_name);
        names[n] = malloc(len + 4);
        memcpy(names[n] + 3, e->d_name, len + 1);
        names[n][0] = (char)('0' + e->d_type / 10);
        names[n][1] = (char)('0' + e->d_type % 10);
        names[n][2] = ' ';
        n++;
    }
    int end_errno = errno;
    qsort(names, (size_t)n, sizeof *names, by_name);
    for (int i = 0; i < n; i++) {
        if (!quiet || i < 3 || i >= n - 3)
            printf("  %s\n", names[i]);
        free(names[i]);
    }
    (void)types;
    printf("opendir(%s) entries=%d end errno=%d closedir=%d\n", path, n, end_errno, closedir(d));
}

int main(void) {
    char start[4096], here[4096], want[4200];
    char *g = getcwd(start, sizeof start);
    printf("getcwd ok=%d absolute=%d\n", g == start, start[0] == '/');
    SHOW("mkdir sub", mkdir("sub", 0755));
    SHOW("chdir sub", chdir("sub"));
    getcwd(here, sizeof here);
    snprintf(want, sizeof want, "%s/sub", start);
    printf("cwd is start/sub=%d\n", strcmp(here, want) == 0);
    int fd = open("inside", O_CREAT | O_WRONLY, 0644);
    close(fd);
    SHOW("chdir ..", chdir(".."));
    getcwd(here, sizeof here);
    printf("cwd is start=%d\n", strcmp(here, start) == 0);
    SHOW("chdir missing", chdir("missing"));
    SHOW("chdir file", chdir("sub/inside"));
    SHOW("chdir empty", chdir(""));
    errno = 0;
    char *r = getcwd(here, 1);
    printf("getcwd size 1 null=%d errno=%d\n", r == NULL, errno);
    errno = 0;
    r = getcwd(here, 0);
    printf("getcwd size 0 null=%d errno=%d\n", r == NULL, errno);
    size_t need = strlen(start) + 1;
    errno = 0;
    r = getcwd(here, need);
    printf("getcwd exact ok=%d\n", r == here);
    errno = 0;
    r = getcwd(here, need - 1);
    printf("getcwd one short null=%d errno=%d\n", r == NULL, errno);

    /* A small directory, a large one (several reads of the kernel's
     * listing), names up to 255 bytes, a symbolic link and a subdirectory. */
    mkdir("big", 0755);
    char name[300];
    for (int i = 0; i < 1000; i++) {
        snprintf(name, sizeof name, "big/entry-%04d-%s", i, i % 100 == 0 ? "with-a-much-longer-name-to-vary-record-lengths" : "x");
        fd = open(name, O_CREAT | O_WRONLY, 0644);
        close(fd);
    }
    memset(name, 'n', sizeof name);
    memcpy(name, "big/", 4);
    name[4 + 255] = 0;
    fd = open(name, O_CREAT | O_WRONLY, 0644);
    printf("long name fd ok=%d\n", fd >= 0);
    close(fd);
    mkdir("big/subdir", 0755);
    symlink("entry-0001-x", "big/link");
    list("sub", 0);
    list(".", 0);
    list("big", 1);
    list("missing", 0);
    list("sub/inside", 0);
    list("", 0);

    /* Remove what was made, so the two columns' directories compare. */
    DIR *d = opendir("big");
    struct dirent *e;
    int n = 0, removed = 0;
    while ((e = readdir(d)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;
        names[n] = malloc(strlen(e->d_name) + 5);
        snprintf(names[n], strlen(e->d_name) + 5, "big/%s", e->d_name);
        n++;
    }
    closedir(d);
    for (int i = 0; i < n; i++) {
        if (strcmp(names[i], "big/subdir") == 0)
            removed += rmdir(names[i]) == 0;
        else
            removed += unlink(names[i]) == 0;
        free(names[i]);
    }
    int rb = rmdir("big");
    printf("removed=%d rmdir big=%d\n", removed, rb);
    int u = unlink("sub/inside");
    printf("unlink inside=%d rmdir sub=%d\n", u, rmdir("sub"));
    printf("DT_REG=%d DT_DIR=%d DT_LNK=%d DT_UNKNOWN=%d\n", DT_REG, DT_DIR, DT_LNK, DT_UNKNOWN);
    return 0;
}
