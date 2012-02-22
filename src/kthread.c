/**
 * src/kthread.c
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 */

#include "kthread.h"
#include "kmem.h"
#include <unistd.h>


/*****************************************************************************/
/* Globals */

/* This flag is true if the library is operating in multithreaded mode. */
int ktools_use_mt = 0;

/* This key is used to create and retrieve the thread-specific object associated
 * to each thread.
 */
static pthread_key_t thread_specific_key;

/* The thread-specific object associated to the main thread. */
static struct kthread_specific *main_thread_specific = NULL;


/*****************************************************************************/
/* Internal functions */

/* Wrappers to avoid checking for fatal errors that we can't handle. The
 * 'pthread' part of the function wrapped is replaced by 'internal' in the
 * wrapper function. Clearly the error handling code must not refer to the
 * thread-specific error buffer.
 */
static void internal_create(pthread_t *thread_id, pthread_attr_t *attr, void * (*fct)(void *), void *arg) {
    char buf[1000];
    int error = pthread_create(thread_id, attr, fct, arg);
    if (error) kerror_fatal("cannot create thread: %s", kerror_sys_buf(error, buf));
}

static void internal_join(pthread_t thread_id) {
    char buf[1000];
    int error = pthread_join(thread_id, NULL);
    if (error) kerror_fatal("cannot join thread: %s", kerror_sys_buf(error, buf));
}

static void internal_attr_init(pthread_attr_t *attr) {
    char buf[1000];
    int error = pthread_attr_init(attr);
    if (error) kerror_fatal("cannot initialize thread attribute: %s", kerror_sys_buf(error, buf));
}

static void internal_attr_destroy(pthread_attr_t *attr) {
    char buf[1000];
    int error = pthread_attr_destroy(attr);
    if (error) kerror_fatal("cannot destroy thread attribute: %s", kerror_sys_buf(error, buf));
}

static void internal_attr_setdetachstate(pthread_attr_t *attr, int value) {
    char buf[1000];
    int error = pthread_attr_setdetachstate(attr, value);
    if (error) kerror_fatal("cannot set thread detach state attribute: %s", kerror_sys_buf(error, buf));
}

static void internal_key_create(pthread_key_t *key, void (*destructor_func)(void *)) {
    char buf[1000];
    int error = pthread_key_create(key, destructor_func);
    if (error) kerror_fatal("cannot create thread key object: %s", kerror_sys_buf(error, buf));
}

static void internal_key_delete(pthread_key_t key) {
    char buf[1000];
    int error = pthread_key_delete(key);
    if (error) kerror_fatal("cannot delete thread key object: %s", kerror_sys_buf(error, buf));
}

static void internal_setspecific(pthread_key_t key, void *value) {
    char buf[1000];
    int error = pthread_setspecific(key, value);
    if (error) kerror_fatal("cannot set thread-specific value: %s", kerror_sys_buf(error, buf));
}

static void internal_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr) {
    char buf[1000];
    int error = pthread_mutex_init(mutex, attr);
    if (error) kerror_fatal("cannot initialize mutex: %s", kerror_sys_buf(error, buf));
}

static void internal_mutex_destroy(pthread_mutex_t *mutex) {
    char buf[1000];
    int error = pthread_mutex_destroy(mutex);
    if (error) kerror_fatal("cannot destroy mutex: %s", kerror_sys_buf(error, buf));
}

static void internal_mutexattr_init(pthread_mutexattr_t *attr) {
    char buf[1000];
    int error = pthread_mutexattr_init(attr);
    if (error) kerror_fatal("cannot initialize mutex attribute: %s", kerror_sys_buf(error, buf));
}

static void internal_mutexattr_destroy(pthread_mutexattr_t *attr) {
    char buf[1000];
    int error = pthread_mutexattr_destroy(attr);
    if (error) kerror_fatal("cannot destroy mutex attribute: %s", kerror_sys_buf(error, buf));
}

static void internal_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
    char buf[1000];
    int error = pthread_mutexattr_settype(attr, type);
    if (error) kerror_fatal("cannot set mutex attribute: %s", kerror_sys_buf(error, buf));
}

static void internal_mutex_lock(pthread_mutex_t *mutex) {
    char buf[1000];
    int error = pthread_mutex_lock(mutex);
    if (error) kerror_fatal("cannot lock mutex: %s", kerror_sys_buf(error, buf));
}

static void internal_mutex_unlock(pthread_mutex_t *mutex) {
    char buf[1000];
    int error = pthread_mutex_unlock(mutex);
    if (error) kerror_fatal("cannot unlock mutex: %s", kerror_sys_buf(error, buf));
}

static struct kthread_specific * kthread_specific_new() {
    struct kthread_specific *self = (struct kthread_specific *) kcalloc(sizeof(struct kthread_specific));
    kerror_init(&self->error_stack);
    kstr_init(&self->err_str);
    return self;
}

static void kthread_specific_destroy(struct kthread_specific *self) {
    if (self) {
	kerror_clean(&self->error_stack);
	kstr_clean(&self->err_str);
	kfree(self);
    }
}

/* This function is called automatically by the pthread API when a thread has
 * finished. It destroys the kthread_specific object.
 */
static void kthread_specific_destructor(void *arg) {
    struct kthread_specific *s = (struct kthread_specific *) arg;
    assert(s);
    kthread_specific_destroy(s);
    internal_setspecific(thread_specific_key, NULL);
}

/* This function calls the user-supplied thread-startup function. */
static void * kthread_start_internal(void *internal_arg) {
    void **arg_array = (void **) internal_arg;
    void (*run_func)(struct kthread *, void *) = (void (*)(struct kthread *, void *)) arg_array[0];
    struct kthread *thread = (struct kthread *) arg_array[1];
    void *user_arg = arg_array[2];
    thread->pid = getpid();
    run_func(thread, user_arg);
    kfree(internal_arg);
    return NULL;
}


/*****************************************************************************/
/* Multithreading control functions */

/* This function makes this library start operating in multi-threaded mode. If
 * this function is not called, this library will operate in single-threaded
 * mode. In this case the functions of the threading module must not be called.
 */
void kthread_enter_mt_mode() {

    /* Create the thread-specific object key and register the main thread
     * thread-specific object.
     */
    assert(main_thread_specific == NULL);
    internal_key_create(&thread_specific_key, kthread_specific_destructor);
    main_thread_specific = kthread_register_thread();
    ktools_use_mt = 1;
}

/* This function makes the library stop operating in multi-threading mode. This
 * means that the memory used by the threading module will be freed. Note that
 * you shouldn't call this function if there are still active threads (foreign
 * or not) using some components of this library: the information is still
 * needed! It is not a big deal if you don't call this function. Valgrind will
 * report that some memory is still reachable. This is not a leak.
 *
 * One note if you use threads created outside this library:
 *
 * If a foreign thread uses the synchronization primitives of this library, the
 * thread will be "registered" by this library. This means that a
 * kthread_specific object will be associated to that thread. The operation
 * should be invisible to the foreign package that created the thread. However,
 * if you call this method after a foreign thread has used a synchronization
 * primitive of this library, some memory will be leaked (unfortunate, but the
 * pthread API works this way). To prevent this from happening, all such foreign
 * threads must explicitely call the function kthread_unregister_thread() before
 * this function is called.
 */
void kthread_exit_mt_mode() {

    /* Unregister the main thread thread-specific object then delete the
     * thread-specific object key.
     */
    assert(main_thread_specific != NULL);
    ktools_use_mt = 0;
    kthread_unregister_thread();
    main_thread_specific = NULL;
    internal_key_delete(thread_specific_key);
}


/*****************************************************************************/
/* struct kthread interface */

/* This function initializes a thread object. */
void kthread_init(struct kthread *self) {
    memset(self, 0, sizeof(struct kthread));
}

/* This function cleans a thread object. */
void kthread_clean(struct kthread *self) {
    assert(! self->running_flag);
}

/* This function starts the thread execution by calling the specified function
 * and passing it this thread object and a user-defined argument.
 */
void kthread_start(struct kthread *self, void (*run_func)(struct kthread *, void *), void *user_arg) {
    pthread_attr_t attr;
    void **arg_array = (void **) kmalloc(3 * sizeof(void *));
    
    arg_array[0] = run_func;
    arg_array[1] = self;
    arg_array[2] = user_arg;
    
    assert(! self->running_flag);
    self->running_flag = 1;
    internal_attr_init(&attr);
    internal_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    internal_create(&self->thread_id, &attr, kthread_start_internal, arg_array);
    internal_attr_destroy(&attr);
}

/* This function joins the specified thread with the calling thread. */
void kthread_join(struct kthread *self) {
    assert(self->running_flag);
    internal_join(self->thread_id);
    assert(self->running_flag);
    self->running_flag = 0;
}

/* This function returns the kthread_specific object associated to the current
 * thread, or NULL if there is none.
 */
struct kthread_specific * kthread_query_specific() {
    return (struct kthread_specific *) pthread_getspecific(thread_specific_key);
}

/* This function returns the kthread_specific object associated to the current
 * thread. If there is no such object, then a new kthread_specific object is
 * allocated, registered and returned.
 */
struct kthread_specific * kthread_get_specific() {
    struct kthread_specific *s = kthread_query_specific();
    if (s) return s;
    return kthread_register_thread();
}

/* This function allocates a kthread_specific object and registers it in the
 * current thread thread-specific memory. The object allocated is returned.
 */
struct kthread_specific * kthread_register_thread() {
    assert(kthread_query_specific() == NULL);
    struct kthread_specific *s = kthread_specific_new();
    internal_setspecific(thread_specific_key, s);
    return s;
}

/* This method removes and destroys the kthread_specific object associated to
 * the current thread, if any.
 */
void kthread_unregister_thread() {
    struct kthread_specific *s = kthread_query_specific();
    if (s) kthread_specific_destructor(s);
}

/* This function returns true if the current thread is the main thread. */
int kthread_is_main_thread() {
    return (kthread_query_specific() == main_thread_specific);
}


/*****************************************************************************/
/* struct kmutex interface */

void kmutex_init(struct kmutex *self) {
    #ifdef NDEBUG
    internal_mutex_init(&self->internal_mutex, NULL);
    #else
    pthread_mutexattr_t attr;
    internal_mutexattr_init(&attr);
    internal_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    internal_mutex_init(&self->internal_mutex, &attr);
    internal_mutexattr_destroy(&attr);
    #endif
}

void kmutex_clean(struct kmutex *self) {
    internal_mutex_destroy(&self->internal_mutex);
}

/* This method locks the mutex. */
void kmutex_lock(struct kmutex *self) {
    internal_mutex_lock(&self->internal_mutex);
}

/* This method unlocks the mutex. */
void kmutex_unlock(struct kmutex *self) {
    internal_mutex_unlock(&self->internal_mutex);
}

