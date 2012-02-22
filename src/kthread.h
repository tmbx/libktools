/**
 * src/kthread.h
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 */
 
#ifndef __KTHREAD_H__
#define __KTHREAD_H__

/* This has to be defined in order for this code to work. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include "kerror.h"

/* This flag is true if the library is operating in multithreaded mode. */
extern int ktools_use_mt;


/* This structure represents a thread created with this library. */
struct kthread {

    /* Private thread ID used by the pthread API. */
    pthread_t thread_id;
    
    /* True if this thread is running. A thread is running if it has been
     * started but not yet joined.
     */
    int running_flag;
    
    /* PID of the thread, when it is running. */
    int pid;
};


/* An instance of this structure is associated to each thread that interacts
 * with the synchronization objects provided by this library. The address of the
 * instance is stored in the thread-specific data of the relevant thread. The
 * instance is purely internal to this library. However, it may get associated
 * to threads created outside this library (foreign threads) when such threads
 * interact with this library. This usage is explicitely supported.
 */
struct kthread_specific {

    /* Buffer used by the annoying strerror_r() function. */
    char buf[1000];
    
    /* Pointer to the error stack used by this thread. */
    struct kerror error_stack;
    
    /* Buffer used to format an error string. */
    kstr err_str;
};


/* A kmutex can be locked to provide mutual exclusion between threads.
 * This mutex does not support recursivity.
 */
struct kmutex {

    /* Pthread mutex object. */
    pthread_mutex_t internal_mutex;
};


void kthread_enter_mt_mode();
void kthread_exit_mt_mode();
void kthread_init(struct kthread *self);
void kthread_clean(struct kthread *self);
void kthread_start(struct kthread *self, void (*run_func)(struct kthread *, void *), void *user_arg);
void kthread_join(struct kthread *self);
struct kthread_specific * kthread_query_specific();
struct kthread_specific * kthread_get_specific();
struct kthread_specific * kthread_register_thread();
void kthread_unregister_thread();
int kthread_is_main_thread();
void kmutex_init(struct kmutex *self);
void kmutex_clean(struct kmutex *self);
void kmutex_lock(struct kmutex *self);
void kmutex_unlock(struct kmutex *self);

#endif
