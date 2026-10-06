/* <string.h> string functions, C17 7.24.6.3 strlen, 7.24.4.2 strcmp,
 * 7.24.4.4 strncmp, 7.24.5.2 strchr, 7.24.2.3 strcpy. */
#include <string.h>
#include "t.h"

static char big[1001];
static char dst[64];

int main(void) {
    t_kv("strlen.empty", (long)strlen(""));
    t_kv("strlen.one", (long)strlen("a"));
    t_kv("strlen.nul_inside", (long)strlen("ab\0cd"));
    for (int i = 0; i < 1000; i++)
        big[i] = (char)('a' + i % 26);
    big[1000] = 0;
    t_kv("strlen.big", (long)strlen(big));
    t_kv("strlen.high", (long)strlen("\xff\x80\x01"));

    t_kv("strcmp.eq", t_sign(strcmp("wolf", "wolf")));
    t_kv("strcmp.empty", t_sign(strcmp("", "")));
    t_kv("strcmp.prefix", t_sign(strcmp("wol", "wolf")));
    t_kv("strcmp.longer", t_sign(strcmp("wolfe", "wolf")));
    t_kv("strcmp.lt", t_sign(strcmp("abc", "abd")));
    t_kv("strcmp.high", t_sign(strcmp("\xff", "a")));

    t_kv("strncmp.zero", t_sign(strncmp("abc", "xyz", 0)));
    t_kv("strncmp.within", t_sign(strncmp("abcX", "abcY", 3)));
    t_kv("strncmp.at", t_sign(strncmp("abcX", "abcY", 4)));
    t_kv("strncmp.past_nul", t_sign(strncmp("ab", "ab", 100)));
    t_kv("strncmp.short", t_sign(strncmp("ab", "abc", 3)));
    t_kv("strncmp.high", t_sign(strncmp("a\x90", "a\x10", 2)));

    const char *s = "muzzle wolf";
    const char *p = strchr(s, 'z');
    t_kv("strchr.first", p ? (long)(p - s) : -1);
    p = strchr(s, 'f');
    t_kv("strchr.last", p ? (long)(p - s) : -1);
    t_kv("strchr.absent", strchr(s, 'q') == NULL);
    p = strchr(s, '\0');
    t_kv("strchr.nul", p ? (long)(p - s) : -1);
    p = strchr(s, 'w' + 256);
    t_kv("strchr.char_conv", p ? (long)(p - s) : -1);
    t_kv("strchr.empty", strchr("", 'a') == NULL);

    memset(dst, 'z', sizeof dst);
    t_kv("strcpy.ret", strcpy(dst, "howl") == dst);
    t_kv("strcpy.len", (long)strlen(dst));
    t_kv("strcpy.after", dst[5]);
    t_str(dst);
    t_str("\n");
    strcpy(dst, "");
    t_kv("strcpy.empty", dst[0]);
    t_kv("strcpy.kept", dst[1]);
    return 0;
}
