/* <time.h> C17 7.27.3.5 strftime in the C locale, with POSIX.1-2024's
 * additions: %s, the 0 and + flags and a minimum field width (XSH
 * strftime), and %z and %Z from tm_gmtoff and tm_zone. Each conversion
 * is printed alone over a set of times chosen for the week-based fields
 * (%U %W %V %G %g at the turns of the year), the 12-hour clock and the
 * century. The result count and the too-small buffer (0, C17 7.27.3.5p8)
 * are printed as well. */
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *const convs[] = {
    "%a", "%A", "%b", "%B", "%c", "%C", "%d", "%D", "%e", "%F", "%g", "%G", "%h", "%H", "%I", "%j", "%m", "%M",
    "%n", "%p", "%r", "%R", "%S", "%t", "%T", "%u", "%U", "%V", "%w", "%W", "%x", "%X", "%y", "%Y", "%z", "%Z",
    "%%", "%s", "%Ec", "%EC", "%Ex", "%EX", "%Ey", "%EY", "%Od", "%Oe", "%OH", "%OI", "%Om", "%OM", "%OS", "%Ou",
    "%OU", "%OV", "%Ow", "%OW", "%Oy", "%+4Y", "%05Y", "%+12F", "%012F", "%+3C", "%03C", "%+6G", "%06G", "%8Y",
    "%1Y", "%+Y", "%0Y", "<%a %d %b %Y %H:%M:%S %z>",
};

int main(void) {
    static const long long ts[] = {
        0, 951782400, 946684799, 946684800, 1704067200, 1703980800, 1703894400, 1735603200, 1735689600, 1735776000,
        1609459200, 1609372800, 1293840000, 1230768000, 1104537600, 1136073600, 1199145600, 1262217600, 1514764800,
        1546214400, 43200, 46800, 39600, 82800, 3600, -2208988800LL, 253402300799LL, -62135596800LL, -62167219200LL,
        -86400LL * 400,
    };
    char buf[256];
    for (size_t i = 0; i < sizeof ts / sizeof *ts; i++) {
        time_t t = (time_t)ts[i];
        struct tm g;
        gmtime_r(&t, &g);
        printf("t=%lld\n", ts[i]);
        for (size_t c = 0; c < sizeof convs / sizeof *convs; c++) {
            memset(buf, 'x', sizeof buf);
            size_t n = strftime(buf, sizeof buf, convs[c], &g);
            printf("  %s -> %zu [%s]\n", convs[c], n, n ? buf : "");
        }
        struct tm l;
        localtime_r(&t, &l);
        size_t n = strftime(buf, sizeof buf, "%c %Z %z", &l);
        printf("  local -> %zu [%s]\n", n, buf);
    }

    /* Built by hand: an offset and a zone of the program's own, a null
     * zone, fields at their limits. */
    struct tm h;
    memset(&h, 0, sizeof h);
    h.tm_year = 124; h.tm_mon = 6; h.tm_mday = 4; h.tm_hour = 13; h.tm_min = 5; h.tm_sec = 9;
    h.tm_wday = 4; h.tm_yday = 185;
    static const long offs[] = {0, 19800, -3600, 45900, -43200, 50400, 1, -59, 86399};
    for (size_t i = 0; i < sizeof offs / sizeof *offs; i++) {
        h.tm_gmtoff = offs[i];
        h.tm_zone = i % 2 ? "ABC" : NULL;
        size_t n = strftime(buf, sizeof buf, "%z %Z", &h);
        printf("hand off=%ld -> %zu [%s]\n", offs[i], n, buf);
    }
    h.tm_zone = "XYZ";
    h.tm_gmtoff = 0;
    static const int years[] = {-1900, -1901, -1899, -1905, -2000, -3900, 0, 99, 100, 8099, 8100, 10000, 2147481747};
    static const char *const yf[] = {"%Y", "%C", "%y", "%G", "%g", "%F", "%+4Y", "%05Y", "%+6C", "%+12F", "%04C"};
    for (size_t i = 0; i < sizeof years / sizeof *years; i++) {
        h.tm_year = years[i];
        printf("year %d:", years[i]);
        for (size_t f = 0; f < sizeof yf / sizeof *yf; f++) {
            size_t n = strftime(buf, sizeof buf, yf[f], &h);
            printf(" %s=%zu[%s]", yf[f], n, buf);
        }
        printf("\n");
    }
    h.tm_year = 124;
    for (int hr = 0; hr < 24; hr += 1) {
        h.tm_hour = hr;
        strftime(buf, sizeof buf, "%H %I %p %r", &h);
        printf("hour %d [%s]\n", hr, buf);
    }

    /* The count, and a buffer exactly too small. */
    h.tm_hour = 13;
    size_t full = strftime(buf, sizeof buf, "%A %B %d", &h);
    printf("full=%zu [%s]\n", full, buf);
    printf("fits=%zu\n", strftime(buf, full + 1, "%A %B %d", &h));
    printf("short=%zu\n", strftime(buf, full, "%A %B %d", &h));
    printf("zero=%zu\n", strftime(buf, 0, "%A", &h));
    printf("empty=%zu\n", strftime(buf, sizeof buf, "", &h));
    printf("plain=%zu [%s]\n", strftime(buf, sizeof buf, "no conversions", &h), buf);
    return 0;
}
