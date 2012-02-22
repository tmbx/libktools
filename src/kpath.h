/**
 * src/kpath.h
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 *
 * Path manipulation support.
 */

#ifndef __KPATH_H__
#define __KPATH_H__

#include <kstr.h>
#include <karray.h>

/* Define the native and the alternative delimiters of the compilation platform. */
#ifdef __WINDOWS__
#define KPATH_NATIVE_DELIM '\\'
#else
#define KPATH_NATIVE_DELIM '/'
#endif

#define KPATH_ALT_DELIM '/'

/* Define the delimiter formats supported:
 * native_alt:   delimiter is either the native or the alternative delimiter of the
 *               compilation platform. You can rely on this value being 0.
 * native:       delimiter is the native delimiter of the compilation platform.
 * unix:         delimiter is the slash ('/').
 * windows:      delimiter is the backslash ('\').
 * windows_alt:  delimiter is either '/' or '\'.
 */
#define KPATH_FORMAT_NATIVE_ALT    0
#define KPATH_FORMAT_NATIVE        1
#define KPATH_FORMAT_UNIX          2
#define KPATH_FORMAT_WINDOWS       3
#define KPATH_FORMAT_WINDOWS_ALT   4

/* This structure represents a decomposed directory path. */
struct kpath_dir {
    
    /* If the path is absolute, this string contains the initial 'C:\ or '/'
     * portion of the path.
     */
    kstr abs_part;
    
    /* This array contains the subcomponents of the path, except the initial
     * portion of the path.
     */
    karray components;
};

void kpath_dir_init(struct kpath_dir *self);
void kpath_dir_clean(struct kpath_dir *self);
void kpath_dir_reset(struct kpath_dir *self);
int kpath_get_platform_format(int format);
int kpath_is_delim(char c, int format);
char kpath_delim(int format);
void kpath_add_delim(kstr *path, int format);
void kpath_split(kstr *path, kstr *dir, kstr *name, kstr *ext, int format);
void kpath_basename(kstr *path, kstr *name, int format);
int kpath_is_absolute(kstr *path, int format);
void kpath_make_absolute(kstr *path, int format);
void kpath_decompose_dir(kstr *path, struct kpath_dir *dir, int format);
void kpath_recompose_dir(kstr *path, struct kpath_dir *dir, int format);
void kpath_simplify_dir(struct kpath_dir *dir);
void kpath_normalize(kstr *path, int absolute_flag, int format);
void kpath_getcwd(kstr *path);

#endif

