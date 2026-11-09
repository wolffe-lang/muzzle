/* A real program: a recursive directory lister (in the spirit of
 * `find -ls` and `tree`), in plain C. It builds a tree of directories,
 * files of several sizes and symbolic links, walks it with opendir,
 * readdir and lstat (entries sorted by name, links not followed),
 * prints each entry's depth, type, size and name with per-directory
 * totals, then removes the tree depth first and checks it is gone. */
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int by_name(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }

struct total { long files, dirs, links, bytes; };

/* The sorted names in a directory, without . and .., or NULL. */
static char **names_in(const char *dir, size_t *count) {
    DIR *d = opendir(dir);
    if (d == NULL) {
        printf("opendir %s: errno=%d\n", dir, errno);
        return NULL;
    }
    size_t n = 0, cap = 8;
    char **v = malloc(cap * sizeof *v);
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;
        if (n == cap) {
            cap *= 2;
            v = realloc(v, cap * sizeof *v);
        }
        size_t len = strlen(e->d_name) + 1;
        v[n] = malloc(len);
        memcpy(v[n], e->d_name, len);
        n++;
    }
    closedir(d);
    qsort(v, n, sizeof *v, by_name);
    *count = n;
    return v;
}

static void walk(const char *dir, int depth, struct total *t) {
    size_t n;
    char **v = names_in(dir, &n);
    if (v == NULL)
        return;
    struct total mine = {0, 0, 0, 0};
    for (size_t i = 0; i < n; i++) {
        size_t len = strlen(dir) + strlen(v[i]) + 2;
        char *path = malloc(len);
        snprintf(path, len, "%s/%s", dir, v[i]);
        struct stat s;
        if (lstat(path, &s) != 0) {
            printf("lstat %s: errno=%d\n", path, errno);
        } else if (S_ISDIR(s.st_mode)) {
            printf("%*s%s/\n", depth * 2, "", v[i]);
            mine.dirs++;
            walk(path, depth + 1, &mine);
        } else if (S_ISLNK(s.st_mode)) {
            char target[256];
            long k = (long)readlink(path, target, sizeof target - 1);
            target[k > 0 ? k : 0] = 0;
            struct stat f;
            int live = stat(path, &f) == 0;
            printf("%*s%s -> %s%s\n", depth * 2, "", v[i], target, live ? "" : " (dangling)");
            mine.links++;
        } else {
            printf("%*s%s %lld\n", depth * 2, "", v[i], (long long)s.st_size);
            mine.files++;
            mine.bytes += s.st_size;
        }
        free(path);
        free(v[i]);
    }
    free(v);
    printf("%*s[%s: %ld files, %ld dirs, %ld links, %ld bytes]\n", depth * 2, "", dir, mine.files, mine.dirs, mine.links,
           mine.bytes);
    t->files += mine.files;
    t->dirs += mine.dirs;
    t->links += mine.links;
    t->bytes += mine.bytes;
}

static long removed;
static void remove_tree(const char *dir) {
    size_t n;
    char **v = names_in(dir, &n);
    if (v == NULL)
        return;
    for (size_t i = 0; i < n; i++) {
        size_t len = strlen(dir) + strlen(v[i]) + 2;
        char *path = malloc(len);
        snprintf(path, len, "%s/%s", dir, v[i]);
        struct stat s;
        if (lstat(path, &s) == 0 && S_ISDIR(s.st_mode))
            remove_tree(path);
        else if (unlink(path) == 0)
            removed++;
        free(path);
        free(v[i]);
    }
    free(v);
    if (rmdir(dir) == 0)
        removed++;
}

static void file(const char *path, long size) {
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    char buf[512];
    for (long done = 0; done < size;) {
        long k = size - done < (long)sizeof buf ? size - done : (long)sizeof buf;
        memset(buf, 'a' + (int)(done % 26), (size_t)k);
        done += (long)write(fd, buf, (size_t)k);
    }
    close(fd);
}

int main(void) {
    char path[256];
    mkdir("root", 0755);
    for (int a = 0; a < 4; a++) {
        snprintf(path, sizeof path, "root/dir%d", a);
        mkdir(path, 0755);
        for (int b = 0; b < a + 1; b++) {
            snprintf(path, sizeof path, "root/dir%d/sub%c", a, 'a' + b);
            mkdir(path, 0755);
            for (int c = 0; c < 3; c++) {
                snprintf(path, sizeof path, "root/dir%d/sub%c/file%d.dat", a, 'a' + b, c);
                file(path, (long)(a * 1000 + b * 100 + c * 7));
            }
        }
        snprintf(path, sizeof path, "root/dir%d/readme", a);
        file(path, 12345L * a);
    }
    mkdir("root/empty", 0755);
    file("root/top.txt", 1);
    symlink("dir1/suba/file0.dat", "root/link-to-file");
    symlink("dir2", "root/link-to-dir");
    symlink("nowhere", "root/dangling");
    struct total t = {0, 0, 0, 0};
    printf("root/\n");
    walk("root", 1, &t);
    printf("total: %ld files, %ld dirs, %ld links, %ld bytes\n", t.files, t.dirs, t.links, t.bytes);
    remove_tree("root");
    errno = 0;
    printf("removed=%ld gone=%d errno=%d\n", removed, access("root", F_OK) != 0, errno);
    return 0;
}
