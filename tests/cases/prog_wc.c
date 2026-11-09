/* A real program: wc, in plain C. It counts lines, words, bytes and the
 * longest line of its standard input (through stdio) and of three files
 * it writes itself (through open and read), then prints them with a
 * total, in wc's column shape. Words are runs of bytes other than the
 * C locale's white space. */
#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct counts { long lines, words, bytes, longest; };

static int space(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; }

struct state { struct counts c; int in_word; long line; };

static void feed(struct state *s, const unsigned char *p, size_t n) {
    for (size_t i = 0; i < n; i++) {
        int ch = p[i];
        s->c.bytes++;
        if (ch == '\n') {
            s->c.lines++;
            if (s->line > s->c.longest)
                s->c.longest = s->line;
            s->line = 0;
        } else {
            s->line += ch == '\t' ? 8 - s->line % 8 : 1;
        }
        if (space(ch)) {
            s->in_word = 0;
        } else if (!s->in_word) {
            s->in_word = 1;
            s->c.words++;
        }
    }
}

static void finish(struct state *s) {
    if (s->line > s->c.longest)
        s->c.longest = s->line;
}

static void print(const struct counts *c, const char *name) {
    printf("%7ld %7ld %7ld %7ld %s\n", c->lines, c->words, c->bytes, c->longest, name);
}

static int count_file(const char *path, struct counts *out) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror(path);
        return -1;
    }
    struct state s = {{0, 0, 0, 0}, 0, 0};
    unsigned char *buf = malloc(4096);
    long n;
    while ((n = (long)read(fd, buf, 4096)) > 0)
        feed(&s, buf, (size_t)n);
    free(buf);
    close(fd);
    finish(&s);
    *out = s.c;
    return 0;
}

static void make(const char *path, int lines, int seed) {
    FILE *f = fopen(path, "w");
    unsigned x = (unsigned)seed;
    for (int i = 0; i < lines; i++) {
        x = x * 1103515245u + 12345u;
        int words = (int)(x >> 16) % 12;
        for (int w = 0; w < words; w++) {
            x = x * 1103515245u + 12345u;
            int len = 1 + (int)(x >> 16) % 9;
            for (int k = 0; k < len; k++)
                fputc('a' + (int)((x >> (k + 3)) % 26), f);
            fputc(w % 5 == 4 ? '\t' : ' ', f);
        }
        fputc('\n', f);
    }
    fputs("no newline at the end", f);
    fclose(f);
}

int main(void) {
    struct state in = {{0, 0, 0, 0}, 0, 0};
    unsigned char chunk[100];
    size_t k = 0;
    int ch;
    while ((ch = getchar()) != EOF) {
        chunk[k++] = (unsigned char)ch;
        if (k == sizeof chunk) {
            feed(&in, chunk, k);
            k = 0;
        }
    }
    feed(&in, chunk, k);
    finish(&in);
    struct counts total = in.c;
    print(&in.c, "-");
    make("one.txt", 10, 1);
    make("two.txt", 1000, 2);
    make("three.txt", 50000, 3);
    static const char *const files[] = {"one.txt", "two.txt", "three.txt", "missing.txt"};
    for (size_t i = 0; i < sizeof files / sizeof *files; i++) {
        struct counts c;
        if (count_file(files[i], &c) != 0)
            continue;
        print(&c, files[i]);
        total.lines += c.lines;
        total.words += c.words;
        total.bytes += c.bytes;
        if (c.longest > total.longest)
            total.longest = c.longest;
    }
    print(&total, "total");
    return 0;
}
