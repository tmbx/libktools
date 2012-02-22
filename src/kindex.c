/* Kindex
 * This is a wrapper around the khash to serialize/deserialize a mapping
 * between ids (int 32bits) and serializable elements. The serializable destroy
 * function will be called automatically on the elements when a kindex is
 * destroyed. Thus, the elements should not be allocated on the stack.
 */

#include <kindex.h>
#include <stdint.h>
#include <kbuffer.h>
#include <kerror.h>
#include <kmem.h>

static const uint8_t KINDEX_FORMAT_VERSION = 1;

int kindex_serialize(kserializable *serializable, kbuffer *buffer) {
    kindex *self = (kindex *)serializable;
    struct khash_iter hash_iter;
    kiter *iter = (kiter *)&hash_iter;
    struct khash_cell *cell;

    kbuffer_write8(buffer, KINDEX_FORMAT_VERSION);
    kbuffer_write32(buffer, self->hash.size);
    
    khash_iter_init(&hash_iter, &self->hash);
    while (kiter_next(iter, (void **)&cell) == 0) {
        kbuffer_write32(buffer, (uint32_t)*(int *)cell->key);
        if (kserializable_serialize((kserializable *)cell->value, buffer))
            return -1;
    }
    return 0;
}

int kindex_deserialize(kserializable *serializable, kbuffer *buffer) {
    kindex *self = (kindex *)serializable;
    uint8_t version;
    uint32_t nb_elem;
    unsigned int i;

    do {
        if (kbuffer_read8(buffer, &version)) {
            KTOOLS_ERROR_PUSH("could not read khash version");
            break;
        }
        if (version != KINDEX_FORMAT_VERSION) {
            KTOOLS_ERROR_PUSH("unknown khash format");
            break;
        }
        if (kbuffer_read32(buffer, &nb_elem)) {
            KTOOLS_ERROR_PUSH("cannot read the number of element");
            break;
        }

        for (i = 0 ; i < nb_elem ; i++) {
            uint32_t key;
            kserializable *value = NULL;

            if (kbuffer_read32(buffer, &key)) {
                KTOOLS_ERROR_PUSH("could not read key");
                break;
            }
            if (kserializable_deserialize(&value, buffer)) {
                KTOOLS_ERROR_PUSH("could not deserialize the value associated with the key %u", key);
                break;
            }

            if (kindex_add(self, key, value)) {
                kserializable_destroy(value);
                KTOOLS_ERROR_PUSH("there are 2 elements with the same key");
                break;
            }
        }
        if (i < nb_elem)
            break;
        return 0;
    } while (0);
    
    kindex_reset(self);
    return -1;
}

kserializable *kindex_new_serializable() {
    return (kserializable *)kindex_new();
}

void kindex_destroy_serializable(kserializable *serializable) {
    kindex_destroy((kindex *)serializable);
}

void kindex_dump(kserializable *serializable, FILE *file) {
    kindex *self = (kindex *)serializable;
    struct khash_iter hash_iter;
    kiter *iter = (kiter *)&hash_iter;
    struct khash_cell *cell;

    fprintf(file, "hash :\n{\n");

    khash_iter_init(&hash_iter, &self->hash);
    while (kiter_next(iter, (void **)&cell) == 0) {
        fprintf(file, " %u : ", (uint32_t)*(int *)cell->key);
        kserializable_dump((kserializable *)cell->value, file);
        fprintf(file, "\n");
    }
    fprintf(file, "}\n");
}

DECLARE_KSERIALIZABLE_OPS(kindex) = {
    KSERIALIZABLE_TYPE_KINDEX,
    kindex_serialize,
    kindex_deserialize,
    kindex_new_serializable,
    kindex_destroy_serializable,
    kindex_dump,
};

kindex *kindex_new() {
    kindex *self = kmalloc(sizeof(kindex));
    kindex_init(self);
    return self;
}

void kindex_init(kindex *self) {
    khash_init(&self->hash);
    kserializable_init((kserializable *)self, &KSERIALIZABLE_OPS(kindex));
}

void kindex_destroy(kindex *self) {
    if (self)
        kindex_clean(self);
    kfree(self);
}

void kindex_clean(kindex *self) {
    kindex_reset(self);
    khash_clean(&self->hash);
}

int kindex_add(kindex *self, uint32_t key, kserializable *value) {
    int *k = kmalloc(sizeof(int));
    *k = (int)key;
    if (khash_add(&self->hash, k, value)) {
        kfree(k);
        return -1;
    }
    return 0;
}

int kindex_get(kindex *self, uint32_t key, kserializable **rvalue) {
    int k = (int)key;
    return khash_get(&self->hash, &k, NULL, (void **)rvalue);
}

void kindex_reset(kindex *self) {
    struct khash_iter hash_iter;
    kiter *iter = (kiter *)&hash_iter;
    struct khash_cell *cell;

    khash_iter_init (&hash_iter, &self->hash);
    while (kiter_next(iter, (void **)&cell) == 0) {
        kfree(cell->key);
        kserializable_destroy(cell->value);
    }
}
