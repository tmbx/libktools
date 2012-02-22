/**
 * ktime.c
 * Copyright (C) 2006-2012 Opersys inc., All rights reserved.
 *
 * Manipulation of time, mostly with struct timeval.
 */

#include <ktime.h>
#include <kerror.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Return the number of seconds elapsed since the UNIX epoch. */
int64_t ktime_now_sec() {
    return (int64_t) time(NULL);
}

/* Return the number of milliseconds elapsed since the UNIX epoch. */
int64_t ktime_now_msec() {
    struct timeval tv;
    ktime_now(&tv);
    return ktime_to_msec(&tv);
}

/* This function puts the current time in the timeval passed in the parameters.
 * Arguments:
 * Timeval.
 */
void ktime_now(struct timeval *tv) {
    #ifdef __WINDOWS__
    /* Get the number of 100-nanosecond intervals since January 1, 1601 (UTC). */
    uint64_t nb_100nsec_1601;
    uint64_t nb_100nsec_1970;
    GetSystemTimeAsFileTime((struct _FILETIME *) &nb_100nsec_1601);

    /* Get the number of 100-nanoseconds between January 1, 1601 and 1970, January 1.
     * Magic number explanation:
     * Both epochs are Gregorian. 1970 - 1601 = 369. Assuming a leap
     * year every four years, 369 / 4 = 92. However, 1700, 1800, and 1900
     * were NOT leap years, so 89 leap years, 280 non-leap years.
     * 89 * 366 + 280 * 365 = 134744 days between epochs. Of course
     * 60 * 60 * 24 = 86400 seconds per day, so 134744 * 86400 =
     * 11644473600 = SECS_BETWEEN_EPOCHS.
     */
    nb_100nsec_1970 = nb_100nsec_1601 - (11644473600ll * 10000000ll);

    tv->tv_sec = nb_100nsec_1970 / 10000000;
    tv->tv_usec = (nb_100nsec_1970 % 10000000) / 10;
    
    #else
    if (gettimeofday(tv, NULL) != 0)
        kerror_fatal("Cannnot gettimeofday, check capabilities: %s", strerror(errno));
    #endif
}

/* This function returns -1 if first comes before second, 0 if the times are the
 * same and 1 if first comes after second.
 * Arguments:
 * Timeval 1.
 * Timeval 2.
 */
int ktime_cmp(struct timeval *first, struct timeval *second) {
    if (first->tv_sec < second->tv_sec)
    	return -1;

    else if (first->tv_sec > second->tv_sec)
    	return 1;

    else if (first->tv_usec < second->tv_usec)
	return -1;

    else if (first->tv_usec > second->tv_usec)
	return 1;
	    
    else
	return 0;
}

/* This function subtracts y from x and put the result in result.
 * Arguments:
 * Result (can be one of the sources).
 * Timeval to subtract from.
 * Timeval containing the time to subtract.
 */	 
void ktime_sub(struct timeval *result, struct timeval *x, struct timeval *y) {
    int real_sec = x->tv_sec - y->tv_sec;
    int real_usec = x->tv_usec - y->tv_usec;

    /* Perform the carry for the subtraction. */
    if (real_usec < 0) {
	    real_sec -= 1;
	    real_usec += 1000000;
    }

    result->tv_sec = real_sec;
    result->tv_usec = real_usec;
}

/* Complement of above function. */
void ktime_add(struct timeval *result, struct timeval *x, struct timeval *y) {
    int real_sec = x->tv_sec + y->tv_sec;
    int real_usec =  x->tv_usec + y->tv_usec;

    /* Perform the carry for the addition. */
    if (real_usec >= 1000000) {
	    real_sec += 1;
	    real_usec -= 1000000;
    }

    result->tv_sec = real_sec;
    result->tv_usec = real_usec;
}

/* This function calculates the time elapsed since 'start' was set and puts the
 * result in 'result'.
 * Arguments:
 * Start timeval.
 * Result timeval.
 */
void ktime_elapsed(struct timeval *result, struct timeval *since) {
    
    /* Get current time. */
    struct timeval current_tv;
    ktime_now(&current_tv);

    /* Do the subtraction. */
    ktime_sub(result, &current_tv, since);
}

/* This function returns the number of milliseconds that a timeval represents. */
int64_t ktime_to_msec(struct timeval *tv) {
    return ((int64_t)tv->tv_sec * 1000 + (int64_t)tv->tv_usec / 1000);
}

/* This function sets the number of milliseconds specified in a timeval struct.
 * Arguments:
 * Timeval.
 * time in milliseconds.
 */
void ktime_from_msec(struct timeval *tv, int64_t ms) {
    tv->tv_sec = ms / 1000;
    tv->tv_usec = (ms % 1000) * 1000;
}

/* Return the sum of the current time and the delay specified, in milliseconds. */
int64_t ktime_set_deadline(int64_t delay) {
    return ktime_now_msec() + delay;
}

/* Compute the difference between the the deadline specified and the current
 * time, in milliseconds. Return true if the difference is negative, i.e. the
 * deadline expired.
 */
int ktime_check_deadline(int64_t deadline, int64_t *remaining) {
    *remaining = deadline - ktime_now_msec();
    return (*remaining < 0);
}

/* This function transforms the time 't' (as returned by time()) into a string
 * understandable by the user, in localtime.
 * Arguments:
 * Time to transform (seconds elapsed from UNIX Epoch).
 * String that will contain the result.
 * tz gmt or localtime ?
 */
void time_sprint(time_t t, kstr *str, enum ktime_tz tz) {
    struct tm *tm;
    if (tz == KTIME_GMT)
        tm = gmtime(&t);
    else
        tm = localtime(&t);

    tm->tm_year += 1900;
    tm->tm_mon += 1;

    kstr_sf(str, "%.2d/%.2d/%.4d %.2d:%.2d:%.2d", tm->tm_mday, tm->tm_mon, tm->tm_year,
						  tm->tm_hour, tm->tm_min, tm->tm_sec);
}

