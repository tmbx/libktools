/**
 * src/karray.c
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 */

#include <string.h>
#include "karray.h"
#include "kmem.h"
#include "kutils.h"
#include "kerror.h"

karray * karray_new() {
    karray *self = (karray *) kmalloc(sizeof(karray));
    karray_init(self);
    return self;

}

void karray_destroy(karray *self) {
    karray_clean(self);
    kfree(self);
}

void karray_init(karray *self) {
    self->alloc_size = 0;
    self->size = 0;
    self->data = NULL;
}

void karray_init_karray(karray *self, karray *init_array) {
    self->alloc_size = init_array->size;
    self->size = init_array->size;
    self->data = kmalloc(self->size * sizeof(void *));
    memcpy(self->data, init_array->data, self->size * sizeof(void *));
}

void karray_reset(karray *self) {
    self->size = 0;
}

void karray_clean(karray *self) {
    
    if (self == NULL)
    	return;

    kfree(self->data);
}

void karray_grow(karray *self, size_t min_len) {
    if (min_len > self->alloc_size) {
    
        /* Compute the snapped size for a given requested size. By snapping to powers
         * of 2 like this, repeated reallocations are avoided.
         */
        if (min_len < 4) {
            self->alloc_size = 4;
        }
        
        else {
	    self->alloc_size = next_power_of_2(min_len);
        }
        
        assert(self->alloc_size >= min_len);    
        self->data = krealloc(self->data, self->alloc_size * sizeof(void *));
    }
}

void karray_push(karray *self, void *elem) {
    karray_set(self, self->size, elem);
}

void *karray_pop(karray *self) {
    if (self->size == 0) {
        KTOOLS_ERROR_SET("trying to pop en empty array");
        return NULL;
    }
    self->size -= 1;
    return self->data[self->size];
}

void *karray_get_top(karray *self) {
    if (self->size == 0) {
        KTOOLS_ERROR_SET("there is no element at the top of the stack");
        return NULL;
    }
    return self->data[self->size -1];
}

void karray_set(karray *self, int pos, void *elem) {
    assert(pos >= 0);

    /* Make sure the array is big enough. */
    karray_grow(self, pos + 1);

    /* Increase the array size to contain at least this position. */
    if (self->size < pos + 1) {
    	self->size = pos + 1;
    }

    self->data[pos] = elem;
}

/* Like in python, karray_get(-2) returns the second last element */
void *karray_get(karray *self, int pos) {
    int eff_pos = pos;
    if (eff_pos < 0)
        eff_pos = self->size - eff_pos;

    if (eff_pos < 0 || eff_pos >= self->size) {
        KTOOLS_ERROR_SET("element %i is out of range [%d, %d]", pos, 0, self->size);
        return NULL;
    }

    return self->data[eff_pos];
}

void karray_assign_karray(karray *self, karray *assign_array) {
    self->size = assign_array->size;
    karray_grow(self, self->size);
    memcpy(self->data, assign_array->data, self->size * sizeof(void *));    
}

void karray_append_karray(karray *self, karray *append_array) {
    karray_grow(self, self->size + append_array->size);
    memcpy(self->data + self->size, append_array->data, append_array->size * sizeof(void *));
    self->size += append_array->size;
}

/******************
 * karray iterator
 ******************/

static void karray_iter_begin(kiter *iter);
static int karray_iter_prev(kiter *iter);
static int karray_iter_next(kiter *iter);
static void karray_iter_end(kiter *iter);
static int karray_iter_get(kiter *iter, void **el);
static int karray_iter_remove(kiter *iter);
static int karray_iter_insert(kiter *iter, void *el);
static int karray_iter_insert_after(kiter *iter, void *el);
static int karray_iter_change(kiter *iter, void *el);
//static struct kiter *karray_iter_copy(kiter *iter);

static struct kiter_ops karray_iter_ops = {
    karray_iter_begin,
    karray_iter_prev,
    karray_iter_next,
    karray_iter_end,
    karray_iter_get,
    karray_iter_remove,
    karray_iter_insert,
    karray_iter_insert_after,
    karray_iter_change,
};

static inline int karray_iter_is_end(kiter *iter) {
    struct karray_iter *self = (struct karray_iter *)iter;
    if (self->pos >= self->array->size) {
        self->pos = self->array->size;
        return 1;
    }
    return 0;
}

static void karray_iter_begin(kiter *iter) {
    struct karray_iter *self = (struct karray_iter *)iter;
    self->pos = -1;
}
static int karray_iter_prev(kiter *iter) {
    struct karray_iter *self = (struct karray_iter *)iter;
    self->pos--;
    if (self->pos < 0 ) {
        self->pos = -1;
        return -1;
    }
    return 0;
}
static int karray_iter_next(kiter *iter) {
    struct karray_iter *self = (struct karray_iter *)iter;
    self->pos++;
    if (self->pos >= self->array->size) {
        self->pos = self->array->size;
        return -1;
    }
    return 0;
}

static void karray_iter_end(kiter *iter) {
    struct karray_iter *self = (struct karray_iter *)iter;
    self->pos = self->array->size;
}

static int karray_iter_get(kiter *iter, void **el) {
    struct karray_iter *self = (struct karray_iter *)iter;
    *el = NULL;
    if (self->pos < 0)
        return -1;
    if (self->pos >= self->array->size)
        return 1;

    *el = karray_get(self->array, self->pos);
    return 0;
}

static int karray_iter_remove(kiter *iter) {
    struct karray_iter *self = (struct karray_iter *)iter;
    if (self->pos < 0)
        return -1;
    if (self->pos >= self->array->size)
        return 1;

    memmove(self->array->data + self->pos,
            self->array->data + self->pos + 1,
            self->array->size - self->pos - 1);
    return 0;
}

static int karray_iter_insert(kiter *iter, void *el) {
    struct karray_iter *self = (struct karray_iter *)iter;
    if (self->pos < 0)
        return -1;

    karray_grow(self->array, self->array->size + 1);
    memmove(self->array->data + self->pos + 1,
            self->array->data + self->pos,
            self->array->size - self->pos);
    self->array->size++;
    karray_set(self->array, self->pos++, el);
    return 0;
}

static int karray_iter_insert_after(kiter *iter, void *el) {
    struct karray_iter *self = (struct karray_iter *)iter;
    if (self->pos >= self->array->size)
        return 1;

    karray_grow(self->array, self->array->size + 1);
    memmove(self->array->data + self->pos,
            self->array->data + self->pos + 1,
            self->array->size - self->pos - 1);
    self->array->size++;
    karray_set(self->array, self->pos + 1, el);
    return 0;
}

static int karray_iter_change(kiter *iter, void *el) {
    struct karray_iter *self = (struct karray_iter *)iter;
    if (self->pos < 0)
        return -1;
    if (self->pos >= self->array->size)
        return 1;

    karray_set(self->array, self->pos, el);
    return 0;
}

#if 0
/* TODO: insert the iterator in a iterator hash table in the array. */
static struct kiter *karray_iter_copy(kiter *iter) {
    struct kiter *copy = kmalloc(sizeof(struct karray_iter));
    memcpy(copy, iter, sizeof(struct karray_iter));
    return copy;
}

/* TODO: remove the iterator from the iterator hash table in the array. Do not use for now. */
void karray_iter_destroy(struct kiter *self) {
    kfree(self);
}
#endif

/* TODO: insert the iterator in a iterator hash table in the array. */
void karray_iter_init(struct karray_iter *self, karray *array) {
    self->array = array;
    self->pos = -1;
    kiter_init((kiter *)self, &karray_iter_ops);
}
