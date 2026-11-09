#include "t.h"
typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
div_t p2_div(int, int);
ldiv_t p2_ldiv(long, long);
int p2_apply(int (*)(const void *, const void *), const void *, const void *);
void p2_run(void (*)(void), int);
int p2_env_first(void);
unsigned long p2_mul(unsigned long, unsigned long);
extern char **__p2_env;
static int cmp(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }
static int hits;
static void hit(void) { hits++; }
int main(void) {
    div_t d = p2_div(-7, 2);
    t_kv("div.quot", d.quot); t_kv("div.rem", d.rem);
    ldiv_t l = p2_ldiv(-7000000000L, 3);
    t_kv("ldiv.quot", l.quot); t_kv("ldiv.rem", l.rem);
    int a = 5, b = 9;
    t_kv("apply", p2_apply(cmp, &a, &b));
    p2_run(hit, 3);
    t_kv("run.hits", hits);
    static char s[] = "AB";
    static char *env[] = {s, 0};
    __p2_env = env;
    t_kv("env.first", p2_env_first());
    t_kv("mul", (long)p2_mul(4294967295UL, 4294967295UL));
    return 0;
}
