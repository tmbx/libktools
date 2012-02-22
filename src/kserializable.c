#include "kserializable.h"
#include "kbuffer.h"
#include "kerror.h"
#include "kmem.h"

extern const struct kserializable_ops *kserializable_array[];
khash *kserializable_ops_index = NULL;

void kserializable_initialize() {
    int i;
    const struct kserializable_ops *ops = kserializable_array[0];
    kserializable_ops_index = khash_new();

    for (ops = kserializable_array[(i = 0)] ; ops != NULL ; ops = kserializable_array[++i]) {
        kserializable_add_ops(ops);
    }
}

int kserializable_add_ops(const struct kserializable_ops *ops) {
    unsigned int *index = kmalloc(sizeof(unsigned int));
    *index = ops->type;
    return khash_add(kserializable_ops_index, (void *)index, (void *)ops);
}

void kserializable_finalize() {
    struct khash_iter khash_iter;
    kiter *iter = (kiter *)&khash_iter;
    struct khash_cell *cell;

    khash_iter_init(&khash_iter, kserializable_ops_index);
    while (kiter_next(iter, (void **)&cell) == 0)
        kfree(cell->key);

    khash_destroy(kserializable_ops_index);
}

void kserializable_init(kserializable *self, struct kserializable_ops *ops) {
    self->ops = ops;
}

int kserializable_serialize_no_header (kserializable *self, struct kbuffer *buffer) {
    return self->ops->serialize(self, buffer);
}

int kserializable_serialize (kserializable *self, struct kbuffer *buffer) {
    kbuffer tmp_buf;
    kbuffer_init(&tmp_buf);
    int ret = -1;
    
    /* TRY */
    do {
        if (kserializable_serialize_no_header(self, &tmp_buf))
            break;
        kbuffer_write32(buffer, (uint32_t)self->ops->type);
        kbuffer_write32(buffer, tmp_buf.len);
        kbuffer_write_buffer(buffer, &tmp_buf);
        ret = 0;
    } while (0);

    kbuffer_clean(&tmp_buf);
    return ret;
}

/* *self must be initialized */
int kserializable_deserialize_no_header (kserializable *self, struct kbuffer *buffer) {
    return self->ops->deserialize(self, buffer);
}

/* *self will be allocated by ops->deserialized if it's NULL. */
int kserializable_deserialize (kserializable **serializable, struct kbuffer *buffer) {
    uint32_t type;
    uint32_t len;
    uint8_t *ptr;
    kbuffer tmp_buf;
    int ret = -1;
    kserializable *self = *serializable;
    kbuffer_init(&tmp_buf);

    /* Try */
    do {
        /* Get the type. */
        if (kbuffer_read32(buffer, &type)) {
            KTOOLS_ERROR_PUSH("cannot deserialize");
            break;
        }

        if (kbuffer_read32(buffer, &len)) {
            KTOOLS_ERROR_PUSH("cannot deserialize");
            break;
        }

        /* Get the ops. */
        if (self) {
            if (self->ops->type != type) {
                KTOOLS_ERROR_SET("trying to deserialize the wrong type, %lu instead of %lu", self->ops->type, type);
                break;
            }
        } else {
            struct kserializable_ops *ops;
            unsigned int u = (unsigned int)type;
            if (khash_get(kserializable_ops_index, (void *)&u, NULL, (void **)&ops)) {
                buffer->pos += len;
                KTOOLS_ERROR_SET("unknown serializable type %lu", type);
                break;
            } else {
                self = ops->allocate();
            }
        }
        
        /* deserialize */
        ptr = kbuffer_write_nbytes(&tmp_buf, len);
        if (kbuffer_read(buffer, ptr, len)) {
            KTOOLS_ERROR_PUSH("cannot deserialize");
            break;
        }

        ret = self->ops->deserialize(self, &tmp_buf);
    } while (0);

    if (*serializable == NULL) {
        if (ret && self)
            kserializable_destroy(self);
        else
            *serializable = self;
    }

    kbuffer_clean(&tmp_buf);

    return ret;
}

void kserializable_destroy(kserializable *self) {
    if (self)
        self->ops->free(self);
}

void kserializable_dump (kserializable *self, FILE *file) {
    self->ops->dump(self, file);
}
