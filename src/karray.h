/**
 * src/array.h
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 *
 * Minimal implementation of an array object.
 */

#ifndef __K_ARRAY_H__
#define __K_ARRAY_H__

#include <sys/types.h>
#include <kstr.h>
#include <kiter.h>

typedef struct karray {
    
    /* The allocated array size (in terms of elements). */
    size_t alloc_size;
    
    /* The number of elements in the array. */ 
    ssize_t size;
    
    /* The element array. */
    void **data;
} karray;

/* This function allocates and creates an empty array. */
karray * karray_new();

/* This function frees the array data and destroys the array. */
void karray_destroy(karray *self);

/* This function creates an empty array. */
void karray_init(karray *self);

/* This function initializes the array from the karray 'init_array'. */
void karray_init_karray(karray *self, karray *init_array);

/* This function clear the list, sets it to empty */
void karray_reset(karray *self);

/* This function frees the array data. */
void karray_clean(karray *self);

/* This function increases the size of the array so that it may contain at least
 * 'min_len' elements.
 */
void karray_grow(karray *self, size_t min_len);

/* This function adds an element at the end of the array. */
void karray_push(karray *self, void *elem);

/* This function remove the element at the end of the array and returns it. */
void *karray_pop(karray *self);

/* This function returns the element at the end of the array. */
void *karray_get_top(karray *self);

/* This function sets an element at the specified position in the array. */
void karray_set(karray *self, int pos, void *elem);

/* This function return the element at pos */
void *karray_get(karray *self, int pos);

/* This function assigns a karray to this array. */
void karray_assign_karray(karray *self, karray *assign_array);

/* This function appends a karray to this array. */
void karray_append_karray(karray *self, karray *append_array);

/*******************************************/
/* Iterator */

struct karray_iter {
    kiter iter;
    karray *array;
    int pos;
};

void karray_iter_init(struct karray_iter *self, karray *array);

/*******************************************/
/* Utility functions, clear an array of kstr. */
static inline void karray_clear_kstr(karray *array) {
    int i;

    for (i = 0; i < array->size; i++) {
    	kstr_destroy((kstr *) array->data[i]);
    }
	
    array->size = 0;
}

#endif /*__K_ARRAY_H__*/
