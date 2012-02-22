#ifndef __K_ERROR_H__
#define __K_ERROR_H__

#include <kstr.h>
#include <karray.h>
#include <errno.h>
#include <string.h>

/* This has to be defined in order for this code to work. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/**
 * Each time you push an error, a kerror_node instance is pushed on the stack.
 * You shoud not instanciate nodes directly nor delete them directly.
 */
struct kerror_node {

    /* File name where the error occurred. Static C string. */
    char *file;
    
    /* Function name where the error occurred. Static C string. */
    char *function;
    
    /* Line where the error occurred. */
    int line;
    
    /* Module where the error occurred. Each module has a unique integer code.
     * XXX LB: it would probably be best to use a static C string here. In the
     * current scheme some modules are getting conflicting IDs.
     */
    int module;
    
    /* Level of the error. The semantics of 'level' are specific to each module. */
    int level;
    
    /* Error string. */
    kstr text;
};

/* This structure represents an error stack. */
struct kerror {
    karray stack;
};


/* Use the following macro to create a new error node and push it on the
 * stack. Or wrap it in your module so you dont have to specify module and
 * level.
 */
#define KERROR_PUSH(module, level, ...) \
    kerror_push(kerror_node_new(__FILE__, __LINE__, __FUNCTION__, module, level, __VA_ARGS__))

#define KERROR_SET(module, level, ...) ({\
    kerror_reset(); \
    kerror_push(kerror_node_new(__FILE__, __LINE__, __FUNCTION__, module, level, __VA_ARGS__)); \
})

#ifdef BUILDING_KTOOLS
# define KTOOLS_ERROR_PUSH(...) \
    KERROR_PUSH(0, 0, __VA_ARGS__)
# define KTOOLS_ERROR_SET(...) \
    KERROR_SET(0, 0, __VA_ARGS__)
#endif /* BUILDING_KTOOLS */

struct kerror_node *kerror_node_new_v(const char *file, int line, const char *function, int module, int level,
    	    	    	    	      const char *format, va_list args);
void kerror_node_destroy(struct kerror_node *node);
void kerror_init(struct kerror *self);
void kerror_clean(struct kerror *self);
void kerror_reset_stack(struct kerror *self);
void kerror_reset();
struct kerror * kerror_get_current();
void kerror_initialize();
void kerror_finalize();
int kerror_has_error();
void kerror_push(struct kerror_node *node);
kstr *kerror_str();
kstr *kerror_str_n(int n);
void kerror_fatal_set_handler(void (*handler)());
void kerror_fatal(const char *format, ...);
char * kerror_sys(int error);
char * kerror_sys_buf(int error, char buf[1000]);

/* This function calls kerror_sys with errno as its parameter. */
static inline char * kerror_syserror() { return kerror_sys(errno); }

static inline struct kerror_node *kerror_node_new(const char *file, int line, const char *function, int module, int level, 
    	    	    	    	    	    	  const char *format, ...) {
    struct kerror_node *node;
    va_list args;
    va_start(args, format);
    node = kerror_node_new_v(file, line, function, module, level, format, args);
    va_end(args);

    return node;
}

#endif /*__K_ERROR_H__*/
