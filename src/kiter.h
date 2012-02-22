#ifndef __K_ITERATOR_H__
#define __K_ITERATOR_H__
/* This is an abstract class, do not use it directly. Modifying the undelying
 * structure directly or with another iterator while using one of these
 * iterator might break that one. The support for it has been thought, but the
 * use for it has not been shown yet. To implement support for using multiple
 * iterator and modifying the structure directly, we need an hash table
 * containing the iterator hashed by address. That way everytime we modify the
 * structure, we will have to iterate in that hash table and make sure we are
 * not moving/removing the node where an iterator currently is. */

typedef struct kiter kiter;

enum kiter_status {
    KITER_BEG, /* At start, next call to next will give 1st elem. */
    KITER_CUR, /* At current pos, no special behaviour. */
    KITER_END, /* At end, most call will fail. */
};

struct kiter_ops {
    /* Set the current to be before the first element (at start). */
    void (*begin)(kiter *self);
    /* Set the current to be the previous element. Ret -1 at start. */
    int (*prev)(kiter *self);
    /* Set the current to be the next element. Ret 1 at end. */
    int (*next)(kiter *self);
    /* Set the current to be the after last element (at end). */
    void (*end)(kiter *self);
    /* Get the value of the current element. Returns NULL and -1 if current is
     * at start and NULL and 1 if current is at end. */
    int (*get)(kiter *self, void **el);
    /* Remove the current element. Ret -1 if current is at start and 1 if
     * current is at end. */
    int (*remove)(kiter *self);
    /* Insert an element before the current. */
    int (*insert)(kiter *self, void *el);
    /* Insert an element after the current. */
    int (*insert_after)(kiter *self, void *el);
    /* Change the value at current. */
    int (*change)(kiter *self, void *el);
#if 0 /* Add this if needed */
    /* Copy the self into copy */
    struct kiter *(*copy)(kiter *self);
    /* Destroy a kiter. Used for destroying copies in algorithms that dont know
     * the iterator type. */
    void (*destroy)(kiter *self);
#endif 
};

struct kiter {
    /* The operations for this kiter */
    struct kiter_ops *ops;
    /* The value of the current node */
    void *current;
    /* Are we on the current node, after the current node or at the end */
    enum kiter_status status;
};

void kiter_init(kiter *self, struct kiter_ops *ops);
void kiter_begin(kiter *self);
int kiter_prev(kiter *self, void **el);
int kiter_next(kiter *self, void **el);
void kiter_end(kiter *self);
int kiter_get(kiter *self, void **el);
int kiter_remove(kiter *self, void **el);
int kiter_insert(kiter *self, void *el);
int kiter_insert_after(kiter *self, void *el);
int kiter_replace(kiter *self, void **orig_el, void *new_el);
#if 0 /* Add this if needed */
struct kiter *kiter_copy(kiter *self);
void kiter_destroy(kiter *self);
#endif

#if !defined(__BUILDING_KITER) && !defined(__NO_KITER_CAST)
#  define kiter_begin(__S) kiter_begin((kiter *)(__S))
#  define kiter_prev(__S, __E) kiter_prev((kiter *)(__S), (void **)(__E))
#  define kiter_next(__S, __E) kiter_next((kiter *)(__S), (void **)(__E))
#  define kiter_end(__S) kiter_end((kiter *)(__S))
#  define kiter_get(__S, __E) kiter_get((kiter *)(__S), (void **)(__E))
#  define kiter_remove(__S, __E) kiter_remove((kiter *)(__S), (void **)(__E))
#  define kiter_insert(__S, __E) kiter_insert((kiter *)(__S), (void *)(__E_)
#  define kiter_insert_after(__S, __E) kiter_insert_after((kiter *)(__S), (void *)(__E))
#  define kiter_replace(__S, __O, __N) kiter_replace((kiter *)(__S), (void **)(__O), (void *)(__N))
#endif

#endif /*__K_ITERATOR_H__*/
