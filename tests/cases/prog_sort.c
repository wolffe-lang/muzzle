/* A real program: sort, in plain C. It reads its standard input line by
 * line (fgets into buffers grown with realloc), then writes four orders:
 * bytewise (strcmp), numeric by the leading integer (strtol, ties kept
 * in input order: the stable qsort docs/LEDGER.md records), reversed
 * with adjacent duplicates removed (sort -ru), and by the second field.
 * The bytewise order also goes to a file, which the harness compares. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct line { char *text; long key; int has_key; };

static int bytewise(const void *a, const void *b) {
    return strcmp(((const struct line *)a)->text, ((const struct line *)b)->text);
}

static int reverse(const void *a, const void *b) { return bytewise(b, a); }

static int numeric(const void *a, const void *b) {
    const struct line *x = a, *y = b;
    if (x->has_key != y->has_key)
        return x->has_key - y->has_key;
    return (x->key > y->key) - (x->key < y->key);
}

static const char *field2(const char *s) {
    while (*s == ' ' || *s == '\t')
        s++;
    while (*s && *s != ' ' && *s != '\t')
        s++;
    return s;
}

static int second(const void *a, const void *b) {
    return strcmp(field2(((const struct line *)a)->text), field2(((const struct line *)b)->text));
}

static char *read_line(FILE *f) {
    size_t cap = 16, len = 0;
    char *s = malloc(cap);
    for (;;) {
        if (fgets(s + len, (int)(cap - len), f) == NULL) {
            if (len == 0) {
                free(s);
                return NULL;
            }
            return s;
        }
        len += strlen(s + len);
        if (len > 0 && s[len - 1] == '\n') {
            s[len - 1] = 0;
            return s;
        }
        cap *= 2;
        s = realloc(s, cap);
    }
}

static void emit(FILE *f, const char *title, struct line *v, size_t n) {
    fprintf(f, "== %s (%zu)\n", title, n);
    for (size_t i = 0; i < n; i++)
        fprintf(f, "%s\n", v[i].text);
}

int main(void) {
    size_t n = 0, cap = 4;
    struct line *v = malloc(cap * sizeof *v);
    char *s;
    while ((s = read_line(stdin)) != NULL) {
        if (n == cap) {
            cap *= 2;
            v = realloc(v, cap * sizeof *v);
        }
        char *end;
        v[n].text = s;
        v[n].key = strtol(s, &end, 10);
        v[n].has_key = end != s;
        n++;
    }
    struct line *w = malloc(n * sizeof *w + 1);

    memcpy(w, v, n * sizeof *w);
    qsort(w, n, sizeof *w, bytewise);
    emit(stdout, "bytewise", w, n);
    FILE *out = fopen("sorted.txt", "w");
    emit(out, "bytewise", w, n);
    fclose(out);

    memcpy(w, v, n * sizeof *w);
    qsort(w, n, sizeof *w, numeric);
    emit(stdout, "numeric, stable", w, n);

    memcpy(w, v, n * sizeof *w);
    qsort(w, n, sizeof *w, reverse);
    size_t u = 0;
    for (size_t i = 0; i < n; i++)
        if (u == 0 || strcmp(w[u - 1].text, w[i].text) != 0)
            w[u++] = w[i];
    emit(stdout, "reverse, unique", w, u);

    memcpy(w, v, n * sizeof *w);
    qsort(w, n, sizeof *w, second);
    emit(stdout, "by field 2", w, n);

    for (size_t i = 0; i < n; i++)
        free(v[i].text);
    free(v);
    free(w);
    return 0;
}
