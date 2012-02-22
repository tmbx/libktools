/**
 * klib/tbuffer.c
 * Copyright (C) 2006-2012 Opersys inc., All rights reserved.
 *
 * Dynamic typed buffer.
 *
 * @author François-Denis Gonthier
 */

#include <stdlib.h>
#include <string.h>

#include "kmem.h"
#include "kutils.h"
#include "kerror.h"
#include "kbuffer.h"
#include "tbuffer.h"

/** Creates a new tbuffer. */
tbuffer *tbuffer_new(size_t s) {
    tbuffer *tbuf;

    tbuf = kmalloc(sizeof(tbuffer));
    tbuf->del_dbuf = 1;
    tbuf->dbuf = kbuffer_new(s);

    return tbuf;
}

/** Creates a new tbuffer from a kbuffer. */
tbuffer *tbuffer_new_dbuf(kbuffer *dbuf) {
    tbuffer *tbuf;

    tbuf = kmalloc(sizeof(tbuffer));
    tbuf->del_dbuf = 0;
    tbuf->dbuf = dbuf;

    return tbuf;
}

/** Destroys the typed buffer. */
void tbuffer_destroy(tbuffer *self) {
    if (self) {
        if (self->dbuf != NULL && self->del_dbuf)
        kbuffer_destroy(self->dbuf);
    	kfree(self);
    }
}

/** Writes a string in a tbuffer. 
 *
 * Thus function writes NULL strings as 0-length strings.
 */
void tbuffer_write_cstr(tbuffer *self, const char *str) {  
    size_t s;

    kbuffer_write8(self->dbuf, (char)TBUF_STR);
    
    if (str != NULL) {
        s = strlen(str);

        kbuffer_write32(self->dbuf, s);
        kbuffer_write(self->dbuf, (uint8_t *)str, s);
    } 
    /* Write a NULL string as 0-length. */
    else kbuffer_write32(self->dbuf, 0);
}

/** Writes a kpstr in a tbuffer. 
 *
 * This function writes NULL strings as 0-length strings.
 */
void tbuffer_write_str(tbuffer *self, const char *str, size_t str_s) {
    kbuffer_write8(self->dbuf, (char)TBUF_STR);

    if (str != NULL) {
        kbuffer_write32(self->dbuf, str_s);        
        kbuffer_write(self->dbuf, (uint8_t *)str, str_s);
    } 
    /* Write a NULL string as 0-length. */
    else kbuffer_write32(self->dbuf, 0);
}

/** Writes an unsigned integer in the tbuffer. */
void tbuffer_write_uint32(tbuffer *self, uint32_t n) {
    kbuffer_write8(self->dbuf, (char)TBUF_UINT32);
    kbuffer_write32(self->dbuf, n);
}

/** Writes an unsigned long integer in the tbuffer. */
void tbuffer_write_uint64(tbuffer *self, uint64_t n) {
    kbuffer_write8(self->dbuf, (char)TBUF_UINT64);
    kbuffer_write64(self->dbuf, n);
}

/** Returns the type of the next element readable in the tbuffer. */
int tbuffer_peek_type(tbuffer *self, enum tbuffer_type *type) {
    size_t n;
    uint8_t t;
    
    n = self->dbuf->pos;
    if (kbuffer_read8(self->dbuf, &t)) {
        KTOOLS_ERROR_PUSH("underflow");
        return -1;
    }
    
    kbuffer_seek(self->dbuf, n, SEEK_SET);
    
    if (t != TBUF_UINT32 && t != TBUF_UINT64 && t != TBUF_STR) {
        KTOOLS_ERROR_PUSH("unexpected type");
        return -1;
    }

    *type = t;

    return -1;
}

/** Returns a string from the tbuffer. 
 * 
 * The pointer returned for the string is valid for the lifetime of the
 * tbuffer only.  The string will not be NULL-terminated.
 */
int tbuffer_read_string(tbuffer *self, const char **str, size_t *str_s) {
    uint8_t t;    
    size_t s;

    do {
        if (kbuffer_read8(self->dbuf, &t)) break;
        if (t != TBUF_STR)
            return -1;

        if (kbuffer_read32(self->dbuf, &s)) break;

        /* Check for 0 length strings. */
        if (s > 0) {
            if (kbuffer_left(self->dbuf) < s)
                break;

            *str = (char *)kbuffer_current_pos(self->dbuf);

            kbuffer_seek(self->dbuf, s, SEEK_CUR);
        } 
        /* Zero-length strings == NULL */
        else 
            *str = NULL;

        *str_s = s;

        return 0;
        
    } while (0);

    KTOOLS_ERROR_PUSH("underflow");
    return -1;
}

/** Returns an unsigned integer from the tbuffer. */
int tbuffer_read_uint32(tbuffer *self, uint32_t *n) {
    uint8_t t;

    do {
        if (kbuffer_read8(self->dbuf, &t)) break;
        if (t != TBUF_UINT32) {
            KTOOLS_ERROR_SET("type error");
            return -1;
        }

        if (kbuffer_read32(self->dbuf, n)) break;

        return 0;

    } while (0);

    KTOOLS_ERROR_PUSH("underflow");
    return -1;
}

/** Returns an unsigned integer from the tbuffer. */
int tbuffer_read_uint64(tbuffer *self, uint64_t *n) {
    uint8_t t;

    do {
        if (kbuffer_read8(self->dbuf, &t)) break;
        if (t != TBUF_UINT64) {
            KTOOLS_ERROR_SET("type error");
            return -1;
        }

        if (kbuffer_read64(self->dbuf, n)) break;

        return 0;

    } while (0);

    KTOOLS_ERROR_PUSH("underflow");
    return -1;
}

/* Returns the underlying kbuffer.
 * 
 * Be wary that if you modify the kbuffer under a tbuffer's nose, you
 * may break some things.  I suggest to use this function only to get
 * some properties of the underlying kbuffer and not to modify it.
 */
kbuffer *tbuffer_get_dbuf(tbuffer *self) {
    return self->dbuf;
}
