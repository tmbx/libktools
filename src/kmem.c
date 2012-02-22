/**
 * utils/kmem.c
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include "kmem.h"

static inline void *kmem_default_kmalloc(size_t count) {
    void *ptr = malloc(count);
    
    if (ptr == NULL) 
        kmem_outofmem();
    return ptr;
}

static inline void *kmem_default_kcalloc(size_t count) {
    void *ptr = calloc(1, count);
    
    if (ptr == NULL) 
        kmem_outofmem();
    
    return ptr;
}

static inline void *kmem_default_krealloc(void *ptr, size_t count) {
    ptr = realloc(ptr, count);
    
    if (ptr == NULL) 
        kmem_outofmem();
    
    return ptr;
}

static void kmem_default_outofmem(__attribute__ ((unused)) void *ptr) {
    fprintf(stderr, "out of memory");
    exit(1);
}

static void *(*_kmalloc)(size_t) = kmem_default_kmalloc;
static void *(*_kcalloc)(size_t) = kmem_default_kcalloc;
static void *(*_krealloc)(void *, size_t) = kmem_default_krealloc;
static void (*_kfree)(void *) = free;
static void (*_koutofmem)(void *) = kmem_default_outofmem;
static void *_koutofmem_user_data;

/* Passing Null for any handler sets it to the default handler */
void kmem_set_handler(void *(*_malloc)(size_t),
                     void *(*_calloc)(size_t),
                     void *(*_realloc)(void *, size_t),
                     void (*_free)(void *),
                     void (*_outofmem)(void *),
                     void *_outofmem_user_data) {
    if (_malloc == NULL &&
        _calloc == NULL &&
        _realloc == NULL &&
        _free == NULL &&
        _outofmem == NULL &&
        _outofmem_user_data == NULL) { /* Reset all handlers */

        _kmalloc = kmem_default_kmalloc;
        _kcalloc = kmem_default_kcalloc;
        _krealloc = kmem_default_krealloc;
        _kfree = free;
        _koutofmem = kmem_default_outofmem;
        _koutofmem_user_data = NULL;
    } else { /* Set the new handler or keep the old one. */
        _kmalloc = _malloc ? _malloc : _kmalloc;
        _kcalloc = _calloc ? _calloc : _kcalloc;
        _krealloc = _realloc ? _realloc : _krealloc;
        _kfree = _free ? _free : _kfree;
        _koutofmem = _outofmem ? _outofmem : _koutofmem;
        _koutofmem_user_data = _outofmem_user_data ? _outofmem_user_data : _koutofmem_user_data;
    }
}

void *kmalloc(size_t s) {
    return _kmalloc(s);
}
void *kcalloc(size_t s) {
    return _kcalloc(s);
}
void *krealloc(void *p, size_t s) {
    return _krealloc(p, s);
}
void kfree(void *p) {
    _kfree(p);
}

void kmem_outofmem() {
    _koutofmem(_koutofmem_user_data);
}

