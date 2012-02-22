/**
 * utils/mem.h
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 */

#include <sys/types.h>

#ifndef __K_MEM_H__
#define __K_MEM_H__

void *kmalloc(size_t s);
void *kcalloc(size_t s);
void *krealloc(void *p, size_t s);
void kfree(void *p);
void kmem_outofmem();

/* Sending NULL as a handler keeps the current handler/value. Sending all NULL values
 * resets all handlers. */
void kmem_set_handler(void *(*_malloc)(size_t),
                      void *(*_calloc)(size_t),
                      void *(*_realloc)(void *, size_t),
                      void (*_free)(void *),
                      void (*_outofmem)(void *),
                      void *outofmem_user_data);

#endif /*__K_MEM_H__*/
