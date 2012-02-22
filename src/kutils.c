/* Copyright (C) 2006-2012 Opersys inc., All rights reserved. */

#include "ktools.h"

/* Clone the C string specified. */
char* kutil_strdup(char *s) {
    int l = strlen(s) + 1;
    char *r = kmalloc(l);
    memcpy(r, s, l);
    return r;
}

/* This function implements a portable version of strcasestr(). */
char * kutil_strcasestr(const char *haystack, const char *needle) {
    size_t needle_len = strlen(needle);
    const char *last_start = haystack + strlen(haystack) - needle_len;

    while (haystack <= last_start) {
        if (portable_strncasecmp(haystack, needle, needle_len) == 0)
            return (char *) haystack;

        haystack++;
    }

    return NULL;
}

/* This function looks for 'needle' inside 'haystack' like in
 * portable_strcasestr(), except that the function scans backward inside
 * 'haystack' until either 'needle' is found or 'start' is passed.
 */
char * kutil_reverse_strcasestr(const char *start, const char *haystack, const char *needle) {
    size_t needle_len = strlen(needle);

    while (haystack >= start) {
        if (portable_strncasecmp(haystack, needle, needle_len) == 0)
            return (char *) haystack;

        haystack--;
    }

    return NULL;
}

/* This function dumps the content of a buffer on the stream specified, in
 * ASCII. A newline is inserted after 20 characters have been printed on a line.
 */
void kutil_dump_buf_ascii(unsigned char *buf, int n, FILE *stream) {
    int i;

    for (i = 0; i < n; i++) {
        if (i > 0 && i % 20 == 0) fprintf(stream, "\n");
        else if (i % 20) fprintf(stream, " ");

        if (buf[i] == '\n') fprintf(stream, "\\n");
        else if (buf[i] == '\r') fprintf(stream, "\\r");
        else fprintf(stream, "%c ", buf[i]);
    }
}

/* This function converts an ISO-8859-1 string to an UTF8 string. */
void kutil_latin1_to_utf8(kstr *name) {
    int i;
    kstr tmp;
    kstr_init(&tmp);

    for (i = 0; i < name->slen; i++) {
        unsigned char latin1_char = name->data[i];

        if (latin1_char < 128) {
            kstr_append_char(&tmp, latin1_char);
        }

        else {
            unsigned char utf8_char[2] = { 0xC0 | ((latin1_char & 0xC0) >> 6), 0x80 | (latin1_char & 0x3F) };
            kstr_append_char(&tmp, utf8_char[0]);
            kstr_append_char(&tmp, utf8_char[1]);
        }
    }

    kstr_assign_kstr(name, &tmp);
    kstr_clean(&tmp);
}

/* This function dumps the content of a buffer on the stream specified, in
 * hexadecimal. A newline is inserted after 16 bytes have been printed on a
 * line.
 */
void kutil_dump_buf_hex(unsigned char *buf, int n, FILE *stream) {
    int i;

    for (i = 0; i < n; i++) {
        if (i > 0 && i % 16 == 0) fprintf(stream, "\n");
        fprintf(stream, "%.2x", buf[i]);
    }

    printf("\n");
}

/* This function generates 'len' bytes of random data.
 * This function sets the error string. It returns -1 on failure.
 */
int kutil_generate_random(char *buf, int len) {
    int error = 0;

    #ifdef __WINDOWS__

    /* Don't use this code: it doesn't work on some machines where the
     * cryptographic layer is broken.
     *
     * Update: Ah, but openssl requires that call to work.
     * Gcrypt also, but I disabled it.
     *
     * Update2: Openssl *does not* require the call to work. It masks
     * failures. If CryptAcquireContext has to fail, it takes a lot of
     * time to fail if the last argument is 0, but it fails fast if
     * CRYPT_VERIFYCONTEXT or CRYPT_NEWKEYSET is used. Openssl doesn't
     * attempt to create the context (it's buggy in that respect), but
     * it uses CRYPT_VERIFY_CONTEXT so it fails fast. We disable call
     * to CryptAcquireContext for now to avoid problems.
     */

    #if 0
    HCRYPTPROV hCryptProv;
    if (! CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        if (! CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
            kmo_seterror("cannot acquire crypt context (error %u)", GetLastError());
            return -1;
        }
    }

    error = CryptGenRandom(hCryptProv, len, (BYTE *) buf);
    CryptReleaseContext(hCryptProv, 0);

    if (error == 0) {
        kmo_seterror("cannot generate random data");
        return -1;
    }
    return 0;
    #else
    /* Sigh. Better than nothing, I guess. */
    int i;
    DWORD seed = GetTickCount();
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    seed ^= (DWORD) li.LowPart;
    seed ^= (DWORD) li.HighPart;
    srand((unsigned int) seed);

    for (i = 0; i < len; i++) {
        buf[i] = rand() % 256;
    }

    return error;
    #endif

    #else
    FILE *file_ptr = NULL;

    /* Try. */
    do {
        error = kfs_fopen(&file_ptr, "/dev/urandom", "rb");
        if (error) break;

        error = kfs_fread(file_ptr, buf, len);
        if (error) break;

        error = kfs_fclose(&file_ptr, 0);
        if (error) break;

    } while (0);

    kfs_fclose(&file_ptr, 1);
    return error;
    #endif
}

/** Return a string of random letters of randomized case. */
int kutil_generate_alpha_random(char *buf, size_t len) {
    size_t i;

    if (kutil_generate_random(buf, len) < 0)
        return -1;

    for (i = 0; i < len; i++) {
        if (buf[i] % 2 == 0) {
            buf[i] = abs(buf[i] - 65) % 26 + 65;
        }
        else {
            buf[i] = abs(buf[i] - 97) % 26 + 97;
        }
    }

    return 0;
}

/* This function generates a random double number between 0 and 1 (inclusive). */
double kutil_get_random_double() {
    int num_int = rand();
    return (double) num_int / (double) RAND_MAX;
}

/* This function generates a random integer number between 0 (included) and max
 * (included).
 */
int kutil_get_random_int(int max) {
    double num_double = kutil_get_random_double();
    int num_int = (int) (num_double * (max + 1));
    if (num_int > max) num_int = max;
    return num_int;
}

/** Returns 1 if if the passed string has binary characters.
 *
 * This function uses simple heuristics to determine if a string is
 * printable on the screen.  It simple looks for caracters < 32 but
 * !13 or !10.
 */
int kutils_string_is_binary(const char *str, size_t n) {
    uint32_t i;

    for (i = 0; i < n; i++) {
        if (!((uint8_t)str[i] == 13 || (uint8_t)str[i] == 10)
            && (uint8_t)str[i] < 32)
            return 1;
    }

    return 0;
}

/** Uncleverly wrap a block of text.  XXX: THIS FUNCTION IS NOT USED
 RIGHT SO IT WAS PROBABLY NOT TESTED FOR ALL CORNER CASES. */
void kutils_wrap(const size_t line_len,
                 const char *in, size_t in_s, char *out, size_t *out_s) {
    const char *p;
    size_t o, cl;

    p = in;
    cl = 0;

    /* Nothing to do! At least out_s must be set. */
    if (!out_s) return;

    /* This scans the input string until:
       - the defined limit of the input string [in + in_s]
       - the NULL at the end of the input string */
    for (p = in, o = 0; *p != 0 && p != (in + in_s); p++, cl++, o++) {
        /* If the current line is bigger than line_len - 1, add a new
           line and reset the line length counter. */
        if (cl == line_len - 1 && *p != '\n') {
            if (out) out[o] = '\n';
            o++;
            cl = 0;
        }
        /* Reset the line length counter at the end of a line. */
        if (*p == '\n') cl = 0;

        /* In all case, copy an input character to the output string. */
        if (out) out[o] = *p;
    }

    if (out) out[o] = '\0';
    *out_s = o + 1;
}
/* This function compares two 32 bits integers. */
int kutil_uint32_cmp(void *a, void *b) {
    uint32_t x = *(uint32_t *) a;
    uint32_t y = *(uint32_t *) b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

/* This function compares two 64 bits integers. */
int kutil_uint64_cmp(void *a, void *b) {
    uint64_t x = *(uint64_t *) a;
    uint64_t y = *(uint64_t *) b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}
