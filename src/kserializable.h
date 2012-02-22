/* Interface for the serializable objects */

#ifndef __KC_SERIALIZABLE_H__
#define __KC_SERIALIZABLE_H__

#include <stdio.h>

#define DECLARE_KSERIALIZABLE_OPS(type) struct kserializable_ops type##_serializable_ops
#define KSERIALIZABLE_OPS(type) type##_serializable_ops

#include <khash.h>

typedef struct kserializable kserializable;
struct kbuffer;

/* This is the enumeration of all the serializable types in existences. Reserve
 * them by adding them here and commiting. */
enum kserializable_type_id {
    /* Module libktools */
    KSERIALIZABLE_TYPE_NONE,
    KSERIALIZABLE_TYPE_KBUFFER,
    KSERIALIZABLE_TYPE_KSTR,
    KSERIALIZABLE_TYPE_KINDEX,

    /* Module libkcrypt */
    KSERIALIZABLE_TYPE_KCSYMKEY = (1 << 8),
    KSERIALIZABLE_TYPE_KCPKEY,
    KSERIALIZABLE_TYPE_KCSKEY,
    KSERIALIZABLE_TYPE_KCSIGNATURE,

};

/* This is the interface one must implement to define a serializable type. It
 * must then be added to the index. */
struct kserializable_ops {
    enum kserializable_type_id type;
    int (*serialize) (kserializable *self, struct kbuffer *buffer);
    /* self must be allocate if *self == NULL. Otherwise assume it's allocated and initialized. */
    int (*deserialize) (kserializable *self, struct kbuffer *buffer);
    kserializable *(*allocate) ();
    void (*free) (kserializable *self);
    void (*dump) (kserializable *self, FILE *file);
};

/* This is a mapping between type ids and serializable_ops. It'll be populized
 * by kserializable_initialized with ktools serializable elements. Users should
 * khash_add their own at startup. */
extern khash *kserializable_ops_index;

/* This function add elements to the above index */
int kserializable_add_ops(const struct kserializable_ops *ops);

/* Initialize/finalize the module */
void kserializable_initialize();
void kserializable_finalize();


/* Add this to a struct as the first element to implement the serializable
 * interface */
struct kserializable {
    struct kserializable_ops *ops;
};

/* Wrap the virtual methods. Use these and not the ops directly. */
void kserializable_init(kserializable *self, struct kserializable_ops *ops);

int kserializable_serialize(kserializable *self, struct kbuffer *buffer);
int kserializable_deserialize(kserializable **self, struct kbuffer *buffer);

int kserializable_serialize_no_header(kserializable *self, struct kbuffer *buffer);
int kserializable_deserialize_no_header(kserializable *self, struct kbuffer *buffer);

void kserializable_destroy(kserializable *self);

void kserializable_dump(kserializable *self, FILE *file);

static inline enum kserializable_type_id kserializable_type(kserializable *self) {
    return self->ops->type;
}

#endif /*__KC_SERIALIZABLE_H__*/
