/* serializable hash
 */

#ifndef __K_SER_HASH_H__
#define __K_SER_HASH_H__

#include <khash.h>
#include <kserializable.h>
#include <inttypes.h>

typedef struct kindex {
    kserializable serializable;
    khash hash;
} kindex;

kindex * kindex_new();
void kindex_init(kindex *self);
void kindex_destroy(kindex *self);
void kindex_clean(kindex *self);
int kindex_add(kindex *self, uint32_t key, kserializable *value);
int kindex_get(kindex *self, uint32_t key, kserializable **rvalue);
static inline int kindex_has(kindex *self, uint32_t key) {
    return kindex_get(self, key, NULL) == 0;
}
void kindex_reset(kindex *self);

#endif /*__K_SER_HASH_H__*/
