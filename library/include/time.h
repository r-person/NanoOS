#ifndef _TIME_H
#define _TIME_H

#include <stdint.h>

typedef int64_t time_t;
typedef int64_t clock_t;

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct timeval {
    time_t tv_sec;
    long tv_usec;
};

#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 1

time_t time(time_t *timer);

int clock_gettime(
    int clock_id,
    struct timespec *tp
);

uint32_t time_ms(void);

#endif