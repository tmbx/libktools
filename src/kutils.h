/* Copyright (C) 2006-2012 Opersys inc., All rights reserved. */

#ifndef __K_UTILS_H__
#define __K_UTILS_H__

#include <inttypes.h>
#include <ctype.h>

/* Return the number of elements in a static array of pointers. */
#define KUTIL_ARRAY_SIZE(NAME) (sizeof(NAME) / sizeof(void *))

/* Enable this to redefine assert() so that it causes a segmentation fault on 
 * assertion failure. This allows you to get a stack trace with valgrind.
 */
#ifndef NDEBUG
# undef assert
# include <stdio.h>
# define assert(X) if(! (X)) { printf("Assertion failure at file %s, line %d.\n", __FILE__, __LINE__); *(int *) 0 = 0; }
#endif

#ifdef __WINDOWS__
# define portable_strncasecmp _strnicmp
#else
# define portable_strncasecmp strncasecmp       
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
# define ntohll(x)   (x)
# define htonll(x)   (x)
#else
# define ntohll(x) ({ uint64_t _x = x; \
                    ((((uint64_t)ntohl((uint32_t) _x)) << 32) | \
                     ( (uint64_t)ntohl((uint32_t)(_x >> 32)))); \
                    })
# define htonll(x) ntohll(x)
#endif

#ifndef MIN
#define MIN(a,b) ({ \
     typeof(a) _a = (a); \
     typeof(b) _b = (b); \
     _a < _b ? _a : _b; \
  })
#endif
#ifndef MAX
# define MAX(a,b) ({ \
     typeof(a) _a = (a); \
     typeof(b) _b = (b); \
     _a > _b ? _a : _b; \
  })
#endif

#if ! defined(DEPRECATED) && defined(__GNUC__)
# define DEPRECATED __attribute((deprecated))
#elif ! defined(DEPRECATED)
# define DEPRECATED
#endif

#if ! defined(UNUSED) && defined(__GNUC__)
# define UNUSED(x) __attribute((unused)) x
#elif ! defined(UNUSED)
# define UNUSED(x) x
#endif

/* Format to use to print the value of a 64 bits integer. Follow with 'd', 'u',
 * etc.
 */
#ifdef __WINDOWS__
#define PRINTF_64 "%I64"
#else
#define PRINTF_64 "%ll"
#endif


/* This function returns true if the character specified is a digit. */
static inline int is_digit(char c) { return (c >= '0' && c <= '9'); }

/* This function returns the power of 2 just over the value specified. This
 * function assumes that your system is at least 32 bits and that you use will
 * not use values larger than 4GB.
 */
static inline int next_power_of_2(int val)
{
    /* Fill every bits at right of the leftmost set bit:
     * 0001 0000 0000
     * 0001 1000 0000
     * 0001 1110 0000
     * 0001 1111 1110
     * 0001 1111 1111
     *
     * Then add one:
     * 0010 0000 0000
     *
     * So only the next bit to the left of the leftmost bit set is set after 
     * those operations.
     */
    val |= (val >>  1);
    val |= (val >>  2);
    val |= (val >>  4);
    val |= (val >>  8);
    val |= (val >> 16);
    val += 1;

    return val;
}

/* This function puts all the characters of a string in lowercase. */
static inline void strntolower(char *str, size_t max_len) {
    for (; max_len-- > 0 && *str; str++)
        *str = tolower(*str);
}

/* Forward declaration. */
struct kstr;

char* kutil_strdup(char *s);
char * kutil_strcasestr(const char *haystack, const char *needle);
char * kutil_reverse_strcasestr(const char *start, const char *haystack, const char *needle);
void kutil_dump_buf_ascii(unsigned char *buf, int n, FILE *stream);
void kutil_dump_buf_hex(unsigned char *buf, int n, FILE *stream);
void kutil_latin1_to_utf8(struct kstr *name);
int kutil_generate_random(char *buf, int len);
int kutil_generate_alpha_random(char *buf, size_t len);
double kutil_get_random_double();
int kutil_get_random_int(int max);
int kutil_uint32_cmp(void *a, void *b);
int kutil_uint64_cmp(void *a, void *b);
int kutils_string_is_binary(const char *str, size_t n);
void kutils_wrap(const size_t line_len, const char *in, size_t in_s, char *out, size_t *out_s);

#endif /*__K_UTILS_H__*/

