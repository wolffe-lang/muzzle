/* <time.h> clocks: C17 7.27.2.4 time, 7.27.2.1 clock; POSIX.1-2024
 * clock_gettime (CLOCK_REALTIME, CLOCK_MONOTONIC, the CPU-time clocks)
 * and nanosleep. Times are never printed: their relations are (time and
 * CLOCK_REALTIME agree, MONOTONIC never goes back, a sleep lasts at least
 * as long as asked, CPU time grows while the program computes). */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <time.h>

static long long ns(struct timespec t) { return (long long)t.tv_sec * 1000000000LL + t.tv_nsec; }

int main(void) {
    printf("CLOCKS_PER_SEC=%ld\n", (long)CLOCKS_PER_SEC);
    time_t t2 = 0;
    time_t t1 = time(NULL);
    time_t t3 = time(&t2);
    struct timespec rt = {0, -1};
    int r = clock_gettime(CLOCK_REALTIME, &rt);
    time_t t4 = time(NULL);
    printf("time sane=%d stored=%d\n", t1 > 1700000000 && t1 < 4102444800, t3 == t2);
    printf("realtime r=%d between=%d nsec=%d\n", r, rt.tv_sec >= t1 && rt.tv_sec <= t4, rt.tv_nsec >= 0 && rt.tv_nsec < 1000000000);

    static const clockid_t ids[] = {CLOCK_REALTIME, CLOCK_MONOTONIC, CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID};
    for (size_t i = 0; i < sizeof ids / sizeof *ids; i++) {
        struct timespec a = {0, -1};
        errno = 0;
        r = clock_gettime(ids[i], &a);
        printf("clock %zu r=%d errno=%d nsec=%d\n", i, r, errno, a.tv_nsec >= 0 && a.tv_nsec < 1000000000);
    }
    struct timespec bad;
    errno = 0;
    r = clock_gettime((clockid_t)12345, &bad);
    printf("clock bad r=%d errno=%d\n", r, errno);

    struct timespec m0, m1;
    clock_gettime(CLOCK_MONOTONIC, &m0);
    int back = 0;
    for (int i = 0; i < 10000; i++) {
        clock_gettime(CLOCK_MONOTONIC, &m1);
        back |= ns(m1) < ns(m0);
        m0 = m1;
    }
    printf("monotonic never back=%d\n", !back);

    static const long asks[] = {0, 1, 1000, 30000000};
    for (size_t i = 0; i < sizeof asks / sizeof *asks; i++) {
        struct timespec want = {0, asks[i]}, rem = {7, 7};
        clock_gettime(CLOCK_MONOTONIC, &m0);
        errno = 0;
        r = nanosleep(&want, &rem);
        clock_gettime(CLOCK_MONOTONIC, &m1);
        printf("nanosleep %ld r=%d errno=%d long_enough=%d\n", asks[i], r, errno, ns(m1) - ns(m0) >= asks[i]);
    }
    {
        struct timespec want = {1, 0}, rem;
        clock_gettime(CLOCK_MONOTONIC, &m0);
        r = nanosleep(&want, &rem);
        clock_gettime(CLOCK_MONOTONIC, &m1);
        printf("nanosleep 1s r=%d long_enough=%d\n", r, ns(m1) - ns(m0) >= 1000000000LL);
    }
    static const struct timespec badt[] = {{0, 1000000000}, {0, -1}, {-1, 0}};
    for (size_t i = 0; i < sizeof badt / sizeof *badt; i++) {
        errno = 0;
        r = nanosleep(&badt[i], NULL);
        printf("nanosleep bad %zu r=%d errno=%d\n", i, r, errno);
    }

    clock_t c0 = clock();
    volatile unsigned long spin = 0;
    clock_t c1 = c0;
    for (long i = 0; i < 4000000000L && c1 - c0 < 20000; i++) {
        spin += (unsigned long)i;
        if ((i & 0xffff) == 0)
            c1 = clock();
    }
    printf("clock nonneg=%d advanced=%d\n", c0 >= 0, c1 - c0 >= 20000);
    return 0;
}
