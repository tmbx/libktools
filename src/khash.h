/**
 * src/hash.h
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 */

#ifndef __K_HASH_H__
#define __K_HASH_H__

#include <kiter.h>

/* A cell in the hash table: a key and its associated value. */
struct khash_cell {
    void *key;
    void *value;
};

/* The hash itself. */
typedef struct khash {
    
    /* The hashing function used by this hash. This function takes a key object
     * as its argument and returns an integer often unique for that object. By
     * default, we use the value of the pointer as the integer.
     */
    unsigned int (*key_func) (void *);

    /* The comparison function used by this hash. This function takes two key
     * objects as its arguments and returns true if the objects are the same. By
     * default, we compare the values of the pointers.
     */
    int (*cmp_func) (void *, void *);

    /* The table containing the hash cells. */
    struct khash_cell *cell_array;

    /* Size of the table. */
    int alloc_size;

    /* Number of cells used in the table. */
    int size;

    /* Maximum number of cells that can be used before the hash is expanded. */
    int used_limit;

    /* Next index in the prime table to grow the hash. */
    int next_prime_index;

#ifndef NDEBUG
    /* Count the number of collisions. */
    int nb_collision;
#endif
} khash;

khash * khash_new();
void khash_destroy(khash *self);
void khash_init(khash *self);
void khash_init_func(khash *self, unsigned int (*key_func) (void *), int (*cmp_func) (void *, void *));
void khash_set_func(khash *self, unsigned int (*key_func) (void *), int (*cmp_func) (void *, void *));
void khash_clean(khash *self);
void khash_grow(khash *self);
int khash_locate_key(khash *self, void *key);
int khash_add(khash *self, void *key, void *value);
int khash_remove(khash *self, void *key);
int khash_get(khash *self, void *key, void **rkey, void **rvalue);
void khash_reset(khash *self);

struct khash_iter {
    kiter iter;
    khash *hash;
    int pos;
};

void khash_iter_init(struct khash_iter *self, khash *hash);

void khash_iter_next_item(khash *self, int *index, void **key_handle, void **value_handle);
void * khash_iter_next_key(khash *self, int *index);
void * khash_iter_next_value(khash *self, int *index);

/* This function returns true if the key is in the hash.
 * Arguments:
 * Key to look for.
 */
static inline int khash_exist(khash *self, void *key) {
    return (khash_locate_key(self, key) != -1);
}

/* Some hash functions frequently used. */
unsigned int khash_pointer_key(void *key);
int khash_pointer_cmp(void *key_1, void *key_2);
unsigned int khash_cstr_key(void *key);
int khash_cstr_cmp(void *key_1, void *key_2);
unsigned int khash_kstr_key(void *key);
int khash_kstr_cmp(void *key_1, void *key_2);
unsigned int khash_int_key(void *key);
int khash_int_cmp(void *key_1, void *key_2);

#endif /*__K_HASH_H__*/
