/* A binary buffer for reading and writing */
#ifndef __K_BUFFER_H__
#define __K_BUFFER_H__

#include <assert.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <kutils.h>
#include <kserializable.h>

#ifdef __UNIX__
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif

typedef struct kbuffer {
    kserializable serializable;
    uint8_t *data;
    size_t len;
    size_t pos;
    size_t allocated;
#ifndef NDEBUG
    size_t write_max_size;
#endif
} kbuffer;

/* After struct declaration for circular dependency. */
#include "kstr.h"

kbuffer *kbuffer_new();
void kbuffer_destroy(kbuffer *self);

void kbuffer_init(kbuffer *self);
int kbuffer_init_b64(kbuffer *self, kstr *b64);
void kbuffer_clean(kbuffer *self);

void kbuffer_write(kbuffer *self, const uint8_t *data, size_t len);
void kbuffer_write_cstr(kbuffer *self, const char *str);
void kbuffer_write_kstr(kbuffer *self, const kstr *str);
void kbuffer_write_buffer(kbuffer *self, const kbuffer *src);
void kbuffer_writef(kbuffer *self, const char *format, ...);
void kbuffer_writefv(kbuffer *self, const char *format, va_list args);
uint8_t *kbuffer_write_nbytes (kbuffer *self, size_t size);
uint8_t *kbuffer_begin_write(kbuffer *self, size_t max_size);
void kbuffer_end_write(kbuffer *self, size_t size_written);
int kbuffer_eof(kbuffer *self);

void kbuffer_grow(kbuffer *self, size_t size);

static inline void kbuffer_seek(kbuffer *self, ssize_t offset, int whence) {
    switch (whence) {
        case SEEK_SET:
            break;
        case SEEK_CUR:
            offset = self->pos + offset;
            break;
        case SEEK_END:
            offset = self->len + offset;
            break;
        default:
            assert(0);
    }
    
    if (offset < 0)
        self->pos = 0;
    else
        self->pos = MAX(MIN(((size_t)offset), self->len), (size_t)0);
}

int kbuffer_read(kbuffer *self, uint8_t *data, size_t len);

uint8_t *kbuffer_read_nbytes (kbuffer *self, size_t size);

int kbuffer_read_buffer(kbuffer *self, kbuffer *into, uint32_t len);

static inline void kbuffer_write8(kbuffer *self, const uint8_t data) {
    kbuffer_write  (self, &(data), sizeof(uint8_t));
}

static inline void kbuffer_write16 (kbuffer *self, const uint16_t data) {
    uint16_t nbo  = htons(data);
    kbuffer_write  (self, (uint8_t *)&(nbo), sizeof(uint16_t));
}

static inline void kbuffer_write32 (kbuffer *self, const uint32_t data) {
    uint32_t nbo  = htonl(data);
    kbuffer_write  (self, (uint8_t *)&(nbo), sizeof(uint32_t));
}

static inline void kbuffer_write64 (kbuffer *self, const uint64_t data) {
    uint64_t nbo  = htonll(data);
    kbuffer_write  (self, (uint8_t *)&(nbo), sizeof(uint64_t));
}

static inline int kbuffer_read8 (kbuffer *self, uint8_t *data) {
    return kbuffer_read (self, data, sizeof(uint8_t));
}

static inline int kbuffer_read16 (kbuffer *self, uint16_t *data) {
    if (kbuffer_read (self, (uint8_t *)data, sizeof(uint16_t)))
        return -1;
    *data = ntohs(*data);
    return 0;
}

static inline int kbuffer_read32(kbuffer *self, uint32_t *data) {
    if (kbuffer_read (self, (uint8_t *)data, sizeof(uint32_t)))
        return -1;
    *data = ntohl(*data);
    return 0;
}

static inline int kbuffer_read64(kbuffer *self, uint64_t *data) {
    if (kbuffer_read (self, (uint8_t *)data, sizeof(uint64_t)))
        return -1;
    *data = ntohll(*data);
    return 0;
}

static inline void kbuffer_serialize(kbuffer *self, kbuffer *out) {
    kbuffer_write32(out, self->len);
    kbuffer_write_buffer(out, self);
}

static inline int kbuffer_read_serialized(kbuffer *self, kbuffer *into) {
    uint32_t size;
    uint8_t *buf_ptr;

    if (kbuffer_read32(self, &size))
        return -1;
    buf_ptr = kbuffer_write_nbytes(into, size);
    if (kbuffer_read(self, buf_ptr, size))
        return -1;

    return 0;
}

static inline void kbuffer_reset(kbuffer *self) {
    self->len = 0;
    self->pos = 0;
}

static inline uint8_t *kbuffer_current_write_pos(kbuffer *self) {
    return self->data + self->len;
}

static inline size_t kbuffer_left(kbuffer *self) {
    return self->len - self->pos;
}

static inline uint8_t *kbuffer_current_pos(kbuffer *self) {
    return self->data + self->pos;
}

/* This function ensures that the memory pool allocated to the buffer does not
 * get bigger than 'max_size'. When the memory pool is too large, the memory
 * pool is shrunk to 'max_size'. In all cases, both 'pos' and 'len' are set to
 * 0.
 */
static inline void kbuffer_shrink(kbuffer *self, uint32_t max_size) {   
    if (self->allocated > max_size) {
    	kbuffer_clean(self);
	kbuffer_init(self);
    }
    
    self->pos = self->len = 0;
}

#endif /*__K_BUFFER_H__*/
