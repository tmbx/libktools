#include <stdarg.h>
#include "kerror.h"
#include "kmem.h"
#include "kutils.h"
#include "kthread.h"

/* This field is used when the library is operating in single-threaded mode. */
static struct kerror single_thread_instance;

/* Create a new error node from the format provided. Do not call this directly,
 * use the macros that fill the file, function and line.
 */

struct kerror_node *kerror_node_new_v(const char *file, int line, const char *function, int module, int level, const char *format, va_list args) {
    struct kerror_node *node = kmalloc(sizeof(struct kerror_node));

    node->file = (char *)file;
    node->function = (char *)function;
    node->line = line;
    node->module = module;
    node->level = level;
    kstr_init_sfv(&node->text, format, args);

    return node;
}

void kerror_node_destroy(struct kerror_node *node) {
    kstr_clean(&node->text);
    kfree(node);
}

void kerror_init(struct kerror *self) {
    karray_init(&self->stack);
}

void kerror_clean(struct kerror *self) {
    if (self) {
	kerror_reset_stack(self);
	karray_clean(&self->stack);
    }
}

/* This method removes all errors from the specified stack. */
void kerror_reset_stack(struct kerror *self) {
    while (self->stack.size > 0) {
        struct kerror_node *err = (struct kerror_node *) karray_pop(&self->stack);
        kerror_node_destroy(err);
    }
}

/* Same as above, for the current error stack. */
void kerror_reset() {
    kerror_reset_stack(kerror_get_current());
}

/* This function returns a pointer to the current error stack. This function
 * handles multi-threading issues.
 */
struct kerror * kerror_get_current() {
    if (! ktools_use_mt) {
	return &single_thread_instance;
    }
    
    else {
	return &kthread_get_specific()->error_stack;
    }
}

/* initialize/finalize the error module, call at begining/end of program. */
void kerror_initialize() {
    assert(! ktools_use_mt);
    kerror_init(&single_thread_instance);
}

void kerror_finalize() {
    assert(! ktools_use_mt);
    kerror_clean(&single_thread_instance);
}

/* Is there an error ? */
int kerror_has_error() {
    return (kerror_get_current()->stack.size > 0);
}

/* Push an error on the stack. */
void kerror_push(struct kerror_node *node) {
    struct kerror *self = kerror_get_current();
    karray_push(&self->stack, node);
}


static kstr *_kerror_str(int depth) {
    struct kerror *self = kerror_get_current();
    int i;
    kstr *str = kstr_new();

    if (self->stack.size == 0) {
        kstr_append_cstr(str, "unknown error");
        return str;
    }

    if (depth == -1 || depth >= self->stack.size)
        depth = self->stack.size - 1;

    for (i = (signed)depth; i >= 0; i--) {
        struct kerror_node *err = (struct kerror_node *)karray_get(&self->stack, i);
        kstr_append_sf(str, ":: %s:%i (%s) ", err->file, err->line, err->function);
        kstr_append_kstr(str, &err->text);
    }

    kstr_append_char(str, '.');

    return str;
}

/* Convert the error into a string. */
kstr *kerror_str() {
    return _kerror_str(-1);
}

/* Convert a certain number of error on the stack, starting by the bottom. */
kstr *kerror_str_n(int n) {
    return _kerror_str(n);
}

static void (*kerror_fatal_handler)() = NULL;

void kerror_fatal_set_handler(void (*handler)()) {
    kerror_fatal_handler = handler;
}

void kerror_fatal(const char *format, ...) {
    /* Make the fprintf(). */
    va_list arg;
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);

    if (kerror_fatal_handler)
        kerror_fatal_handler();
    
    /* Exit now. */
    abort();
}

/* This function returns a pointer to a string describing the specified C
 * library error code (e.g. from errno). If the library is operating in
 * single-threaded mode, strerror() is used, otherwise strerror_r() is used with
 * the thread-specific error buffer.
 */
char * kerror_sys(int error) {
    if (! ktools_use_mt) {
	return strerror(error);
    }
    
    else {
	return kerror_sys_buf(error, kthread_get_specific()->buf);
    }
}

/* This function formats the error message specified in the buffer specified
 * using strerror_r(). The buffer must be able to contain at least 1000 bytes.
 */
char * kerror_sys_buf(int error, char buf[1000]) {

    /* The documentation of strerror_s() is incorrect. It does not return a
     * string as specified. 
     */
    #ifdef __WINDOWS__
    
    /* The strerror_s() function is not present in any of the MinGW libs and the
     * MinGW developers do not give a shit about it. Since we don't need
     * multithread support on Windows currently, I am using this kludge.
     */
    #if 0
    strerror_s(buf, 1000, error);
    return buf;
    #endif
    sprintf(buf, "error %d", error);
    return buf;
    
    #else
    /* Not using XSI-compliant version, because we're using the GNU interface
     * because pthread doesn't work correctly if we don't. I could elaborate but
     * I don't feel like clarifying this matter.
     */
    strerror_r(error, buf, 1000);
    return buf;
    #endif
}

