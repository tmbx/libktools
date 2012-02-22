/**
 * ktime.h
 * Copyright (C) 2006-2012 Opersys inc., All rights reserved.
 *
 * Manipulation of time, mostly with struct timeval.
 */
#ifndef __K_TIME_H__
#define __K_TIME_H__
#include <sys/time.h>
#include <time.h>
#include <kstr.h>

enum ktime_tz {
    KTIME_GMT,
    KTIME_LOCAL
};

int64_t ktime_now_sec();
int64_t ktime_now_msec();
void ktime_now(struct timeval *tv);
int ktime_cmp(struct timeval *first, struct timeval *second);
void ktime_sub(struct timeval *result, struct timeval *x, struct timeval *y);
void ktime_add(struct timeval *result, struct timeval *x, struct timeval *y);
void ktime_elapsed(struct timeval *result, struct timeval *since);
int64_t ktime_to_msec(struct timeval *tv);
void ktime_from_msec(struct timeval *tv, int64_t ms);
int64_t ktime_set_deadline(int64_t delay);
int ktime_check_deadline(int64_t deadline, int64_t *remaining);
void time_sprint(time_t t, kstr *str, enum ktime_tz tz);

/* Wrap the above function to use a struct timeval for consistency. */
static inline void ktime_sprint(struct timeval *tv, kstr *str, enum ktime_tz tz) {
    time_sprint(tv->tv_sec, str, tz);
}

#endif

