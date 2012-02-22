#include <assert.h>
#include <string.h>
#include "kbuffer.h"
#include "kmem.h"
#include "kutils.h"
#include "base64.h"
#include "kerror.h"

static int kbuffer_serialize_serializable(kserializable *serializable, kbuffer *buffer) {
    kbuffer *self = (kbuffer *)serializable;
    kbuffer_write32(buffer, self->len);
    kbuffer_write(buffer, self->data, self->len);
    return 0;
}

/* The element is not allocated when serializable point to a non-NULL element. */
static int kbuffer_deserialize(kserializable *serializable, kbuffer *buffer) {
    kbuffer *self = (kbuffer *)serializable;
    uint32_t len;
    uint8_t *ptr;

    if (kbuffer_read32(buffer, &len)) {
        KTOOLS_ERROR_SET("not enough data");
        return -1;
    }

    ptr = kbuffer_write_nbytes(self, len);
    if (kbuffer_read(buffer, ptr, len)) {
        KTOOLS_ERROR_SET("not enough data");
        return -1;
    }
    return 0;
}

static kserializable *kbuffer_new_serializable() {
    return (kserializable *)kbuffer_new();
}

static void kbuffer_destroy_serializable(kserializable *self) {
    return kbuffer_destroy((kbuffer *)self);
}

static void kbuffer_dump(kserializable *self, FILE *file) {
    kbuffer b64;
    kbuffer_init(&b64);
    kbin2b64((kbuffer *)self, &b64);
    kbuffer_write8(&b64, '\0');
    fprintf(file, "b64 (%s)", (char *)b64.data);
    kbuffer_clean(&b64);
}

DECLARE_KSERIALIZABLE_OPS(kbuffer) = {
    KSERIALIZABLE_TYPE_KBUFFER,
    kbuffer_serialize_serializable,
    kbuffer_deserialize,
    kbuffer_new_serializable,
    kbuffer_destroy_serializable,
    kbuffer_dump,
};

kbuffer *kbuffer_new() {
    kbuffer *self = kmalloc(sizeof(kbuffer));
    kbuffer_init(self);
    return self;
}

void kbuffer_destroy (kbuffer *self) {
    kbuffer_clean (self);
    kfree (self);
}

void kbuffer_init(kbuffer *self) {
    assert (self);

    self->len = 0;
    self->pos = 0;
    self->allocated = 256;
    self->data = (uint8_t *)kmalloc (self->allocated);
    kserializable_init((kserializable *)self, &KSERIALIZABLE_OPS(kbuffer));
}

int kbuffer_init_b64(kbuffer *self, kstr *b64) {
    int ret = 0;
    kbuffer b64buf;
    kbuffer_init(self);
    kbuffer_init(&b64buf);
    kbuffer_write_kstr(&b64buf, b64);
    if (kb642bin(&b64buf, self, 0)) {
        KTOOLS_ERROR_PUSH("cannot initialize buffer from base64");
        ret = -1;
    }
    kbuffer_clean(&b64buf);
    return ret;
}

void kbuffer_clean(kbuffer *self) {
    if (self)
        kfree (self->data);
}

void kbuffer_grow(kbuffer *self, size_t size) {
    if (self->allocated >= size) return;

    self->allocated = next_power_of_2 (size);
    self->data = krealloc(self->data, self->allocated); 
}

void kbuffer_write(kbuffer *self, const uint8_t *const data, size_t len) {
    uint8_t *ptr = kbuffer_write_nbytes(self, len);
    memcpy(ptr, data, len);
}

void kbuffer_write_cstr(kbuffer *self, const char *str) {
    size_t len = strlen(str);
    kbuffer_write(self, (uint8_t *)str, len);
}

void kbuffer_write_kstr(kbuffer *self, const kstr *str) {
    kbuffer_write(self, (uint8_t *)str->data, str->slen);
}

void kbuffer_write_buffer(kbuffer *self, const kbuffer *src) {
    kbuffer_write(self, src->data, src->len);
}

void kbuffer_writefv(kbuffer *self, const char *format, va_list args) {
    kstr str;
    kstr_init_sfv(&str, format, args);
    kbuffer_write_kstr(self, &str);
    kstr_clean(&str);
}

void kbuffer_writef(kbuffer *self, const char *format, ...) {
    va_list args;
    va_start(args, format);
    kbuffer_writefv(self, format, args);
    va_end(args);
}

uint8_t *kbuffer_write_nbytes (kbuffer *self, size_t size) {
    uint8_t *ptr;
    kbuffer_grow(self, self->len + size);
    ptr = self->data + self->len;
    self->len += size;
    return ptr;
}

uint8_t *kbuffer_begin_write(kbuffer *self, size_t max_size) {
    kbuffer_grow(self, self->len + max_size);
#ifndef NDEBUG
    self->write_max_size = max_size;
#endif
    return self->data + self->len;
}

void kbuffer_end_write(kbuffer *self, size_t size_written) {
    assert(size_written <= self->write_max_size);
    self->len += size_written;
}

int kbuffer_read(kbuffer *self, uint8_t *data, size_t len) {
    if (len > self->len - self->pos) {
        KTOOLS_ERROR_SET("buffer is too short to read %u bytes at bytes %i of %i", len, self->pos, self->len);
	return -1;
    }
    
    memcpy (data, self->data + self->pos, len);
    self->pos += len;
    return 0;
}

uint8_t *kbuffer_read_nbytes (kbuffer *self, size_t size) {
    uint8_t *ptr;
    if (self->len - self->pos < size) {
        KTOOLS_ERROR_SET("buffer is too short to read %u bytes at bytes %i of %i", size, self->pos, self->len);
        return NULL;
    }
    ptr = self->data + self->pos;
    self->pos += size;
    return ptr;
}

int kbuffer_read_buffer(kbuffer *self, kbuffer *into, uint32_t len) {
    if (self->len - self->pos < len) {
        KTOOLS_ERROR_SET("not enough data");
        return -1;
    }
    
    kbuffer_write(into, self->data + self->pos, len);
    self->pos += len;

    return 0;
}

int kbuffer_eof(kbuffer *self) {
    return self->len <= self->pos;
}
