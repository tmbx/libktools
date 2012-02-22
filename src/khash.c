/**
 * src/hash.c
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 */

#include <string.h>
#include "khash.h"
#include "kmem.h"
#include "kstr.h"
#include "kutils.h"
#include "kerror.h"

/* Prime table used to grow the hash. */
static unsigned int khash_prime_table[] = {
    23, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
    196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653,
    100663319, 201326611, 402653189, 805306457, 1610612741, 4294967291u
};

/* The proportion of the cells that must be used before we decide to grow the 
 * hash table.
 */
#define KHASH_FILL_THRESHOLD 0.7

/* This function assumes that pos1 comes before pos2. It returns the distance between the two.
 * It is used internally.
 * Arguments:
 * Position 1.
 * Position 2.
 * Size of the hash (for modulus operation).
 */
inline static int khash_dist(int pos1, int pos2, int base) {
    return (pos1 <= pos2) ? pos2 - pos1 : base - (pos1 - pos2);
}

khash * khash_new() {
    khash *self = (khash *) kmalloc(sizeof(khash));
    khash_init(self);
    return self;

}

void khash_destroy(khash *self) {
    if (self)
        khash_clean(self);
    kfree(self);
}

/* This function initializes the hash. By default keys are hashed and compared
 * by integers.
 */
void khash_init(khash *self) {
    khash_init_func(self, khash_int_key, khash_int_cmp);
}

void khash_init_func(khash *self, unsigned int (*key_func) (void *), int (*cmp_func) (void *, void *)) {
    self->key_func = key_func;
    self->cmp_func = cmp_func;
    self->size = 0;
    self->used_limit = (int) (11 * KHASH_FILL_THRESHOLD);
    self->next_prime_index = 0;
    self->alloc_size = 11;
    self->cell_array = (struct khash_cell *) kcalloc(self->alloc_size * sizeof(struct khash_cell));
#ifndef NDEBUG
    self->nb_collision = 0;
#endif
}

/* This function sets the key hash and compare functions used to hash objects. */
void khash_set_func(khash *self, unsigned int (*key_func) (void *), int (*cmp_func) (void *, void *)) {
    self->key_func = key_func;
    self->cmp_func = cmp_func;
}

/* This function frees the content of the hash. */
void khash_clean(khash *self) {
    if (self == NULL)
    	return;
    
    kfree(self->cell_array);
}

/* This function increases the size of the hash. */
void khash_grow(khash *self) {
    int index;
    int new_alloc_size;
    struct khash_cell *new_cell_array;
    
    /* Get the new size. */
    new_alloc_size = khash_prime_table[self->next_prime_index];
    self->next_prime_index++;
    self->used_limit = (int) (new_alloc_size * KHASH_FILL_THRESHOLD);

    /* Allocate the new table. */
    new_cell_array = (struct khash_cell *) kcalloc(new_alloc_size * sizeof(struct khash_cell));
    
#ifndef NDEBUG
    self->nb_collision = 0;
#endif
    /* Copy the elements. */
    for (index = 0; index < self->alloc_size; index++) {
        void *key = self->cell_array[index].key;
        
        if (key != NULL) {
            /* Put the key at the right place. */
            int i = self->key_func(key) % new_alloc_size;
            
            while (1) {
                /* OK, found an empty slot. */
                if (new_cell_array[i].key == NULL) {
                    /* Set the key / value pair. */
                    new_cell_array[i].key = key;
                    new_cell_array[i].value = self->cell_array[index].value;
                    break;
    	    	}

#ifndef NDEBUG
                self->nb_collision++;
#endif
                i = (i + 1) % new_alloc_size;
    	    }
    	}
    }
    
    /* Free the old table. */
    kfree(self->cell_array);
    
    /* Assign the new table and the new size. */
    self->alloc_size = new_alloc_size;
    self->cell_array = new_cell_array;
}    

/* This function returns the position corresponding to the key in the hash, or -1
 * if it is not there.
 * Arguments:
 * Key to locate.
 */
int khash_locate_key(khash *self, void *key) {
    int index;
    assert(key != NULL);
    
    /* Go where the key should be close. */
    index = self->key_func(key) % self->alloc_size;
        
    while (1) {
        /* Empty slot (the key is not there). */
        if (self->cell_array[index].key == NULL) {
            return -1;
        }
            
        /* Same slot. Must compare key values. */
        else if (self->cmp_func(self->cell_array[index].key, key))
            return index;
        
        /* Not the same key. Advance to the next position, possibly looping back
	 * to the beginning.
	 */
        index = (index + 1) % self->alloc_size;
    }
}

/* This function adds a key / value pair in the hash. If the key is already
 * present, it will be replaced. The key cannot be NULL.
 * Arguments:
 * Key to add.
 * Value to add.
 */
int khash_add(khash *self, void *key, void *value) {
    int index;
    assert(key != NULL);
    
    /* Grow hash if it is too small. */
    if (self->size >= self->used_limit)
        khash_grow(self);

    /* Go where the key should be close. */
    index = self->key_func(key) % self->alloc_size;

    while (1) {
    
        /* Empty slot. It's a new key. */
        if (self->cell_array[index].key == NULL) {
	
            /* Increment use count. */
            self->size++;
            
            /* Set the key / value pair. */
            self->cell_array[index].key = key;
            self->cell_array[index].value = value;
            return 0;
    	}
        
        /* Must compare key values. If they are the same, do not replace them
         * as it will leak memory. */
        if (self->cmp_func(self->cell_array[index].key, key)) {
	    KTOOLS_ERROR_SET("the key is already in the hash");
            return -1;
    	}
        
        /* Not the same key. Advance to the next position, possibly looping back
	 * to the beginning.
	 */
#ifndef NDEBUG
        self->nb_collision++;
#endif
        index = (index + 1) % self->alloc_size;
    }
}

/* This function removes the key / value pair from the hash (if any).
 * Arguments:
 * Key to remove.
 */
int khash_remove(khash *self, void *key) {
    int index = khash_locate_key(self, key);
    int gap_position = index;
    int scanned_pos = index;
    
    /* Key is not present in the hash. */
    if(index == -1)
        return -1;
    
    /* We must ensure that other keys remain contiguous when we remove a key.
     * The situation where we need to move a key up is when the position of the
     * key given by key_func() is farther than the actual position of the key in
     * the hash, counting  from the position of the gap. Picture:
     *
     * The key wants to be here. (Distance between gap and pos wanted is far.)
     * A gap is here.            (So we move the key here.)
     * The key is here.          (Distance between gap and key is close.)
     *
     * In this situation, we don't move:
     * The gap is here.        
     * The key wants to be here. (Distance between gap and pos wanted is short.)
     * The key is here.          (Distance between gap and key is far.)
     *
     * If the gap position matches the wanted pos, we must move the key to fill
     * the gap.
     *
     * So here's the plan:
     * First we locate the key to remove. Removing it causes a gap. We start
     * scanning the keys coming next. If we meet a NULL, we're done. If we meet
     * a key, we check where it wants to be. If it wants to be before the gap,
     * we move it there. Then the gap is now at the position of the key we
     * moved, and we continue at the next position. Otherwise, we just continue 
     * with the next position.
     */
    while (1) {
    	int wanted_pos_dist;
	int key_dist;
	
        /* Scan the next position. */
        scanned_pos = (scanned_pos + 1) % self->alloc_size;

        /* We're done. Just set the gap to NULL. */
        if (self->cell_array[scanned_pos].key == NULL) {
            self->cell_array[gap_position].key = NULL;
            break;
    	}

        /* Calculate the distances. */
        wanted_pos_dist = khash_dist(gap_position,
	    	    	    	     self->key_func(self->cell_array[scanned_pos].key) % self->alloc_size,
				     self->alloc_size);
        key_dist = khash_dist(gap_position, scanned_pos, self->alloc_size);    

        /* Situations where we must move key (and value). */
        if (wanted_pos_dist > key_dist || wanted_pos_dist == 0) {
            self->cell_array[gap_position].key = self->cell_array[scanned_pos].key;
            self->cell_array[gap_position].value = self->cell_array[scanned_pos].value;
            gap_position = scanned_pos;
    	}
    }

    /* Decrement the usage count. */
    self->size--;
    return 0;
}

/* This function returns the value corresponding to the key in value, and 0
 * (success) or -1 (not found).
 * Arguments:
 *  Key   to look for.
 *  rkey  returned key, may be NULL.
 *  value returned value, may be NULL.
 */
int khash_get(khash *self, void *key, void **rkey, void **rvalue) {
    int index = khash_locate_key(self, key);

    if (index == -1)
    	return -1;

    if (rkey)
        *rkey = self->cell_array[index].key;
    if (rvalue)
        *rvalue = self->cell_array[index].value;
    return 0;
}

/* This function clears all entries in the hash. */
void khash_reset(khash *self) {
    self->size = 0;
    self->used_limit = (int) (11 * KHASH_FILL_THRESHOLD);
    self->next_prime_index = 0;
    self->alloc_size = 11;
    self->cell_array = (struct khash_cell *) krealloc(self->cell_array,
    	    	    	    	    	    	    	 self->alloc_size * sizeof(struct khash_cell));
    memset(self->cell_array, 0, self->alloc_size * sizeof(struct khash_cell));
#ifndef NDEBUG
    self->nb_collision = 0;
#endif
}

void khash_iter_begin(kiter *self);
int khash_iter_prev(kiter *self);
int khash_iter_next(kiter *self);
void khash_iter_end(kiter *self);
int khash_iter_get(kiter *self, void **el);
int khash_iter_remove(kiter *self);
int khash_iter_add(kiter *self, void *el);

struct kiter_ops khash_iter_ops = {
    khash_iter_begin,
    khash_iter_prev,
    khash_iter_next,
    khash_iter_end,
    khash_iter_get,
    khash_iter_remove,
    khash_iter_add,
    khash_iter_add,
    khash_iter_add,
};

void khash_iter_begin(kiter *iter) {
    struct khash_iter *self = (struct khash_iter *)iter;
    self->pos = -1;
}

int khash_iter_prev(kiter *iter) {
    struct khash_iter *self = (struct khash_iter *)iter;
    while (self->pos >= 0) {
        self->pos++;
        if (self->pos >= 0 && self->hash->cell_array[self->pos].key != NULL)
            return 0;
    }
    return -1;
}

int khash_iter_next(kiter *iter) {
    struct khash_iter *self = (struct khash_iter *)iter;
    while (self->pos < self->hash->alloc_size) {
        self->pos++;
        if (self->pos < self->hash->alloc_size && self->hash->cell_array[self->pos].key != NULL)
            return 0;
    } 
    return 1;
}

void khash_iter_end(kiter *iter) {
    struct khash_iter *self = (struct khash_iter *)iter;
    self->pos = self->hash->alloc_size;
}

int khash_iter_get(kiter *iter, void **el) {
    struct khash_iter *self = (struct khash_iter *)iter;
    if (0 <= self->pos && self->pos < self->hash->alloc_size) {
        *el = (void *)&self->hash->cell_array[self->pos];
        return 0;
    }
    *el = NULL;
    return -1;
}

/* In the context of iterating over an hash table, there's no meaning to
 * remove/insert/change the current node as the table might get resized or the
 * node might be inserted anywhere thus we'll be lost. */
int khash_iter_remove(kiter *iter) {
    iter = iter;
    return -1;
}

int khash_iter_add(kiter *iter, void *el) {
    iter = iter;
    el = el;
    return -1;
}

void khash_iter_init(struct khash_iter *self, khash *hash)  {
    self->hash = hash;
    self->pos = -1;
    kiter_init(&self->iter, &khash_iter_ops);
}

/* This function sets the key / value pair of the next element in the
 * enumeration. It is safe to call this function even if some of the keys in
 * the hash are invalid, e.g. if you freed the pointers to the objects used as
 * the keys. Be careful not to iterate past the end of the hash.
 * Arguments:
 * Pointer to iterator index, which should be initialized to -1 prior to the 
 *   first call.
 * Pointer to the location where you wish the key to be set; can be NULL.
 * Pointer to the location where you wish the value to be set; can be NULL.
 */
void khash_iter_next_item(khash *self, int *index, void **key_handle, void **value_handle) {
    
    for ((*index)++; *index < self->alloc_size; (*index)++) {
        if (self->cell_array[*index].key != NULL) {
            if (key_handle != NULL)
                *key_handle = self->cell_array[*index].key;
            
            if (value_handle != NULL)
                *value_handle = self->cell_array[*index].value;
    	    
            return;
        }
    }
    
    assert(0);
}

/* Same as above, except that it returns only the next key. */
void * khash_iter_next_key(khash *self, int *index) {

    for ((*index)++; *index < self->alloc_size; (*index)++)
        if (self->cell_array[*index].key != NULL)
            return self->cell_array[*index].key;
    
    assert(0);
    return NULL;
}

/* Same as above, except that it returns only the next value. */
void * khash_iter_next_value(khash *self, int *index) {
    
    for ((*index)++; *index < self->alloc_size; (*index)++)
        if (self->cell_array[*index].key != NULL)
            return self->cell_array[*index].value;
    
    assert(0);
    return NULL;
}


/*******************************************/
/* Utility functions for hashing of frequent types. */

unsigned int khash_pointer_key(void *key) {
    return (unsigned int) (((size_t) key) * 3);
}

int khash_pointer_cmp(void *key_1, void *key_2) {
    return key_1 == key_2;
}

unsigned int khash_cstr_key(void *key) {
    char *str = (char *) key;
    unsigned int value = 0;
    int str_size = strlen(str);
    int index ;

    for(index = 0; index < str_size; index++)
    	value += str[index];

    return value;
}

int khash_cstr_cmp(void *key_1, void *key_2) {
    return (strcmp((char *) key_1, (char *) key_2) == 0);
}

unsigned int khash_kstr_key(void *key) {
    return khash_cstr_key(((kstr *) key)->data);
}

int khash_kstr_cmp(void *key_1, void *key_2) {
    kstr *str_1 = (kstr *) key_1;
    kstr *str_2 = (kstr *) key_2;
    return (str_1->slen == str_2->slen && ! memcmp(str_1->data, str_2->data, str_1->slen));
}

unsigned int khash_int_key(void *key) {
    unsigned int *i = (unsigned *) key;

    return (*i * 3);
}

int khash_int_cmp(void *key_1, void *key_2) {
    return *(unsigned int *) key_1 == *(unsigned int *) key_2;
}
