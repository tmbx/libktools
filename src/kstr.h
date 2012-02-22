/**
 * src/str.h
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 */

#ifndef __K_STR_H__
#define __K_STR_H__

#include <stdarg.h>
#include <kserializable.h>

typedef struct kstr
{
    /* Implement the serializable interface */
    kserializable serializable;

    /* The allocated buffer size. */
    int mlen;
    
    /* The string length, not including the final '0'. */
    int slen;
    
    /* The character buffer, always terminated by a '0'.
     * Note that there may be other '0' in the string.
     */
    char *data;
} kstr;

#include <kbuffer.h>

/* This function allocates and returns an empty kstr. */
kstr* kstr_new();

/* This function allocates and returns a kstr initialized to the string
 * specified.
 */
kstr* kstr_new_kstr(kstr *str);

/* This function allocates and returns a kstr initialized to the string
 * specified.
 */
kstr* kstr_new_cstr(char *str);

/* This function frees the string data and the kstr object. */
void kstr_destroy(kstr *str);

/* This function initializes the string to an empty string. */
void kstr_init(kstr *self);

/* This function initializes the string to the C string 'init_str'. */
void kstr_init_cstr(kstr *self, const char *init_str);

/* This function initializes the string to the kstr 'init_str'. */
void kstr_init_kstr(kstr *self, kstr *init_str);

/* This function initializes the string to the buffer 'buf'. */
void kstr_init_buf(kstr *self, const void *buf, int buf_len);

/* This function initializes the string in the sprintf manner. */
void kstr_init_sf(kstr *self, const char *format, ...);

/* This function initializes the string in the vsprintf manner. */
void kstr_init_sfv(kstr *self, const char *format, va_list args);

/* This function convert every upper case characters into lower case. */
void kstr_tolower(kstr *str);

/* This function frees the string data. */
void kstr_clean(kstr *self);

/* This function increases the size of the memory containing the string so that it
 * may contain at least 'min_slen' characters (not counting the terminating '0').
 */
void kstr_grow(kstr *self, int min_slen);

/* This function assigns the empty string to the string. */
void kstr_reset(kstr *self);

/* This function ensures that the string specified does not get too large. If
 * the internal memory associated to the string is bigger than the threshold
 * specified, the memory associated to the string is released and a new, small
 * buffer is allocated for the string. In all cases, the string is cleared.
 */
void kstr_shrink(kstr *self, int max_size);

/* This function assigns a C string to this string. */
void kstr_assign_cstr(kstr *self, const char *assign_str);

/* This function assigns a kstr to this string. */
void kstr_assign_kstr(kstr *self, kstr *assign_str);

/* This function assigns the content of a raw buffer to the string. */
void kstr_assign_buf(kstr *self, const void *buf, int buf_len);

/* This function appends a character to the string. */
void kstr_append_char(kstr *self, char c);

/* This function appends a C string to the string. */
void kstr_append_cstr(kstr *self, const char *append_str);

/* This function appends a kstr to the string. */
void kstr_append_kstr(kstr *self, kstr *append_str);

/* This function appends a raw buffer to the string (zeros are appended like
 * other characters).
 */
void kstr_append_buf(kstr *self, const void *buf, int buf_len);

/* This function sprintf at the end of the string. */
void kstr_append_sf(kstr *self, const char *format, ...);

/* SYSTEM-DEPENDENT function.
 * This function vsprintf at the end of the string.
 */
void kstr_append_sfv(kstr *self, const char *format, va_list arg);

/* This function allows you to sprintf() directly inside the string. 
 * Arguments: 
 * Format is the usual printf() format, and the following args are the args that
 *   printf() takes.
 */
void kstr_sf(kstr *self, const char *format, ...);

 /* Same as above, but takes a va_list argument.
 */
void kstr_sfv(kstr *self, const char *format, va_list arg);

/* This function extracts a substring from the string and places it in 'mid_str'.
 * Arguments:
 * Source string.
 * String that will contain the substring.
 * Beginning of the substring in this string.
 * Size of the substring.
 */
void kstr_mid(kstr *self, kstr *mid_str, int begin_pos, int size);

/* Replace all occurences of the string 'from' with the string 'to' in the
 * string specified.
 */
void kstr_replace(kstr *self, char *from, char *to);

/* This function returns true if the two strings are the same. */
int kstr_equal_cstr(kstr *first, const char *second);

/* This function returns true if the two strings are the same. */
int kstr_equal_kstr(kstr *first, kstr *second);

#endif /*__K_STR_H__*/
