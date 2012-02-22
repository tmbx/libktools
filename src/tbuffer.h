/**
 * klib/priv/tbuffer.h
 * Copyright (C) 2006-2012 Opersys inc., All rights reserved.
 *
 * Dynamic typed buffer.
 *
 * @author François-Denis Gonthier
 * Reviewed by Laurent Birtz on 22 january 2007.
 */

#ifndef _KLIB_TBUFFER_H
#define _KLIB_TBUFFER_H

typedef struct __tbuffer tbuffer;

struct __tbuffer {
    int del_dbuf;
    kbuffer *dbuf;
};

/** Typed buffer types
 *
 * That enum matches the #define of equivalent types in the KNP
 * protocol definition.
 */
enum tbuffer_type {
    TBUF_UINT32 = 1,
    TBUF_UINT64 = 2,
    TBUF_STR = 3
};
 
tbuffer *tbuffer_new(size_t s);

tbuffer *tbuffer_new_dbuf(kbuffer *dbuf);

void tbuffer_destroy(tbuffer *self);

kbuffer *tbuffer_get_dbuf(tbuffer *self);

int tbuffer_read_uint32(tbuffer *self, uint32_t *n);

int tbuffer_read_uint64(tbuffer *self, uint64_t *n);

int tbuffer_read_string(tbuffer *self, const char **str, size_t *str_s);

int tbuffer_peek_type(tbuffer *self, enum tbuffer_type *type);

void tbuffer_write_uint64(tbuffer *self, uint64_t n);

void tbuffer_write_uint32(tbuffer *self, uint32_t n);

void tbuffer_write_str(tbuffer *self, const char *str, size_t str_s);

void tbuffer_write_cstr(tbuffer *self, const char *str);

#endif // _KLIB_TBUFFER_H
