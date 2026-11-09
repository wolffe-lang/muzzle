/* muzzle: <time.h> (C17 7.27, POSIX.1-2024 XBD time.h), the functions
 * muzzle implements. Hand-written from the standards' synopses.
 *
 * The local zone is UTC: localtime is gmtime with tm_zone "UTC" (gmtime's
 * is "GMT"), mktime is timegm, and TZ is not read (docs/LEDGER.md). */
#ifndef _MUZZLE_TIME_H
#define _MUZZLE_TIME_H

#include <stddef.h>
#include <sys/types.h>

#define CLOCKS_PER_SEC 1000000L

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3

#define TIME_UTC 1

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

/* C17 7.27.1p4's members, then POSIX.1-2024's tm_gmtoff and tm_zone. */
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    long tm_gmtoff;
    const char *tm_zone;
};

clock_t clock(void);
time_t time(time_t *tloc);
time_t mktime(struct tm *timeptr);
time_t timegm(struct tm *timeptr);
struct tm *gmtime(const time_t *timer);
struct tm *gmtime_r(const time_t *restrict timer, struct tm *restrict result);
struct tm *localtime(const time_t *timer);
struct tm *localtime_r(const time_t *restrict timer, struct tm *restrict result);
size_t strftime(char *restrict s, size_t maxsize, const char *restrict format,
                const struct tm *restrict timeptr);
int clock_gettime(clockid_t clock_id, struct timespec *tp);
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);

#endif
