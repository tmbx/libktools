/** * src/kpath.c
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 *
 * Path manipulation support.
 */
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "kerror.h"
#include "kpath.h"

void kpath_dir_init(struct kpath_dir *self) {
    kstr_init(&self->abs_part);
    karray_init(&self->components);
}

void kpath_dir_clean(struct kpath_dir *self) {
    kpath_dir_reset(self);
    kstr_clean(&self->abs_part);
    karray_clean(&self->components);
}

void kpath_dir_reset(struct kpath_dir *self) {
    int i = 0;
    kstr_reset(&self->abs_part);
    for (i = 0; i < self->components.size; i++) kstr_destroy((kstr *) self->components.data[i]);
    karray_reset(&self->components);
}
    
/* This function takes a format as parameter and returns either
 * KPATH_FORMAT_UNIX or KPATH_FORMAT_WINDOWS, depending on the input format and
 * the compilation platform. The code should be clearer than this description.
 */
int kpath_get_platform_format(int format) {
    if (format == KPATH_FORMAT_NATIVE_ALT || format == KPATH_FORMAT_NATIVE) {
        #ifdef __WINDOWS__
        return KPATH_FORMAT_WINDOWS;
        #else
        return KPATH_FORMAT_UNIX;
        #endif
    }
    
    else if (format == KPATH_FORMAT_WINDOWS_ALT) return KPATH_FORMAT_WINDOWS;
    
    return format;
}

/* This function returns true if the character specified is a delimiter
 * according to the format specified.
 */
int kpath_is_delim(char c, int format) {
    if (format == KPATH_FORMAT_NATIVE_ALT) return (c == KPATH_NATIVE_DELIM || c == KPATH_ALT_DELIM);
    if (format == KPATH_FORMAT_NATIVE) return (c == KPATH_NATIVE_DELIM);
    if (format == KPATH_FORMAT_UNIX) return (c == '/');
    if (format == KPATH_FORMAT_WINDOWS) return (c == '\\');
    return (c == '\\' || c == '/');
}

/* This function returns the preferred delimiter of the format specified. */
char kpath_delim(int format) {
    if (format == KPATH_FORMAT_NATIVE_ALT || format == KPATH_FORMAT_NATIVE) return KPATH_NATIVE_DELIM;
    if (format == KPATH_FORMAT_UNIX) return '/';
    return '\\';
}

/* This function adds a trailing delimiter to the string specified if one is not
 * already present and if the string is non-empty.
 */
void kpath_add_delim(kstr *path, int format) {
    if (path->slen && ! kpath_is_delim(path->data[path->slen - 1], format)) {
        kstr_append_char(path, kpath_delim(format));
    }
}

/* This function splits the path into 3 components, directory (with trailing
 * delimiter), file name (without extension), and extension (without '.').
 *
 * The path '/etc/ld.so.conf' will yield directory '/etc/', file name 'ld', and extension 'so.conf'.
 * The path '/etc/' will yield directory '/etc/', file name '', and extension ''.
 * The path '/etc' will yield directory '/', file name 'etc', and extension ''.
 * The path 'foo.' will yield directory './', file name 'foo.' and extension '', i.e. the extension must not be empty.
 * The path '.foo' will yield directory './', file name '.foo' and extension '', i.e. the file name must not be empty.
 * The path '.' will yield directory './', file name '', and extension ''.
 * The path '..' will yield directory '../', file name '', and extension ''.
 * The path '' will yield directory '', file name '', and extension ''.
 * Rule of thumb: If the extension is not empty, then <dir><filename>.<extension> is the path.
 *		  If the extension is empty, then <dir><filename> is the path.
 * Arguments:
 * String containing the path.
 * Directory string pointer, can be NULL.
 * File name string pointer, can be NULL.
 * Extension string pointer, can be NULL.
 */
void kpath_split(kstr *path, kstr *dir, kstr *name, kstr *ext, int format) {
    int i;
    int last_delim_pos = -1;
    int filename_start_pos = 0;
    int first_dot_pos = -1;
    
    /* Locate the last path delimiter, if any. */
    for (i = path->slen - 1; i >= 0; i--) {
        if (kpath_is_delim(path->data[i], format)) {
            last_delim_pos = i;
            break;
        }
    }
    
    /* Check if the next characters after the last delimiter, or the first
     * characters if there is no last delimiter, are exactly and no more than
     * '.' or '..'. In that case, consider the whole path to be a directory
     * path.
     */
    if (last_delim_pos != -1) filename_start_pos = last_delim_pos + 1;
    if (! strcmp(path->data + filename_start_pos, ".")) filename_start_pos += 1;
    else if (! strcmp(path->data + filename_start_pos, "..")) filename_start_pos += 2;
    
    /* Locate the first dot in the file name, if any. */
    for (i = filename_start_pos; i < path->slen; i++) {
        
        /* We found a dot. */
        if (path->data[i] == '.') {
            
            /* If there's nothing before or after the dot, consider there is no dot. */
            if (i != filename_start_pos && i != path->slen - 1) first_dot_pos = i;
            break;
        }
    }

    /* Get the directory portion. */
    if (dir) {
    
        /* Empty path. */
        if (! path->slen) kstr_reset(dir);
        
        /* Relative path with no directory specified. */
        else if (! filename_start_pos) kstr_sf(dir, ".%c", kpath_delim(format)); 
        
        /* Directory present. Extract the directory and add a delimiter if
         * required.
         */
        else {
            kstr_mid(path, dir, 0, filename_start_pos);
            if (! kpath_is_delim(dir->data[dir->slen - 1], format)) kstr_append_char(dir, kpath_delim(format));
        }
    }

    /* Get the file name. */
    if (name) {
        int filename_size;

        /* No extension case. */
        if (first_dot_pos == -1) filename_size = path->slen - filename_start_pos;

        /* Extension case. */
        else filename_size = first_dot_pos - filename_start_pos;
        
        kstr_mid(path, name, filename_start_pos, filename_size);
    }

    /* Get the extension. */
    if (ext) {
    
        /* No extension case. */
        if (first_dot_pos == -1) kstr_reset(ext);

        /* Extension case. */
        else kstr_mid(path, ext, first_dot_pos + 1, path->slen - first_dot_pos - 1);
    }
}

/* This function extracts the file name with the extension from the path
 * specified.
 */
void kpath_basename(kstr *path, kstr *name, int format) {
    int i, last_delim_pos = -1, filename_start_pos = 0;

    /* Locate the last path delimiter, if any. */
    for (i = path->slen - 1; i >= 0; i--) {
        if (kpath_is_delim(path->data[i], format)) {
            last_delim_pos = i;
            break;
        }
    }

    /* Get the position where the file name starts. */
    if (last_delim_pos != -1) filename_start_pos = last_delim_pos + 1;
    
    /* Extract the filename and extension. */
    kstr_mid(path, name, filename_start_pos, path->slen - filename_start_pos);
}

/* This function returns true if a path is an absolute path. */
int kpath_is_absolute(kstr *path, int format) {
    
    /* Get the platform format. */
    int platform_format = kpath_get_platform_format(format);
    
    /* Windows format. Path must start with something like "C:\". */
    if (platform_format == KPATH_FORMAT_WINDOWS)
	return (path->slen >= 3 && path->data[1] == ':' && kpath_is_delim(path->data[2], format));
    
    /* UNIX format. Path must start with "/". */
    else return (path->data[0] == '/');
}

/* This function converts a path to an absolute path, if needed. */
void kpath_make_absolute(kstr *path, int format) {
    kstr tmp;

    /* Return if the path is already absolute. */
    if (kpath_is_absolute(path, format)) return;
    
    kstr_init(&tmp);
    
    /* Get the current working directory and append the relative path to the
     * current working directory.
     */
    kpath_getcwd(&tmp);
    kstr_append_kstr(&tmp, path);
    kstr_assign_kstr(path, &tmp);
    
    kstr_clean(&tmp);
}

/* This function decomposes the directory path specified into an array of
 * subcomponents. If the path is absolute, the absolute part will be something
 * like 'C:\' on Windows and '/' on UNIX, otherwise it will be empty. The
 * remaining components will not contain delimiters.
 *
 * The path 'C:/toto/bar' will yield ('C:\', 'toto', 'bar') on Windows with alt
 * support.
 * The path '/toto/bar/' will yield ('/', 'toto', 'bar') on UNIX.
 * The path './../foo/' will yield ('.', '..', 'foo').
 * The path '' will yield ().
 */
void kpath_decompose_dir(kstr *path, struct kpath_dir *dir, int format) {
    int scan_pos = 0;
    kstr cur_component;
    
    /* Get the platform format. */
    int platform_format = kpath_get_platform_format(format);
    
    /* Determine if the path is absolute. */
    int is_absolute = kpath_is_absolute(path, format);
    
    kstr_init(&cur_component);
    kpath_dir_reset(dir);
    
    /* If the path is absolute, obtain the absolute part. */
    if (is_absolute) {
    
        if (platform_format == KPATH_FORMAT_UNIX) {
            kstr_append_char(&dir->abs_part, '/');
            scan_pos = 1;
        }
        
        else {
            kstr_append_buf(&dir->abs_part, path->data, 2);
            kstr_append_char(&dir->abs_part, kpath_delim(format));
            scan_pos = 3;
        }
    }
    
    /* Scan the rest of the path. */
    while (scan_pos < path->slen) {
        
        /* We encountered a delimiter. */
        if (kpath_is_delim(path->data[scan_pos], format)) {
            
            /* If the current component is empty, ignore it. Otherwise, add the
             * component in the component array.
             */
            if (cur_component.slen) {
                kstr *s = kstr_new();
                kstr_assign_kstr(s, &cur_component);
                karray_push(&dir->components, s);
            }
            
            /* Reset the current component. */
            kstr_reset(&cur_component);
        }
        
        /* Append the current character. */
        else kstr_append_char(&cur_component, path->data[scan_pos]);
        
        scan_pos++;
    }
    
    /* Add the last component, if any, in the component array. */
    if (cur_component.slen) {
        kstr *s = kstr_new();
        kstr_assign_kstr(s, &cur_component);
        karray_push(&dir->components, s);
    }
    
    kstr_clean(&cur_component);
}

/* This function recomposes a directory path from its subcomponents. If the
 * directory path has no absolute part and no other components, the directory
 * path will be empty. Otherwise, the directory path will end with a delimiter.
 */
void kpath_recompose_dir(kstr *path, struct kpath_dir *dir, int format) {
    int i;
    
    kstr_reset(path);
    
    if (dir->abs_part.slen) kstr_assign_kstr(path, &dir->abs_part);
    
    for (i = 0; i < dir->components.size; i++) {
        kstr_append_kstr(path, (kstr *) dir->components.data[i]);
        kstr_append_char(path, kpath_delim(format));
    }
}

/* This function simplifies the directory path specified, removing superflous
 * '..' and '.' components.
 */
void kpath_simplify_dir(struct kpath_dir *dir) {
    karray stack;
    int i, is_abs = dir->abs_part.slen;
    
    karray_init(&stack);
    
    for (i = 0; i < dir->components.size; i++) {
        kstr *s = (kstr *) dir->components.data[i];
        
        /* Going back in the path. */
        if (kstr_equal_cstr(s, "..")) {
            
            /* The stack is empty. */
            if (! stack.size) {
            
                /* If the path is an absolute path, we cannot go back past the
                 * root, so ignore the '..'. Otherwise, add the '..'.
                 */
                if (is_abs) kstr_destroy(s);
                else karray_push(&stack, s);
            }
            
            /* The stack is not empty. */
            else {
                
                /* Get the previous component. */
                kstr *prev = (kstr *) karray_pop(&stack);
                
                /* The previous component is '..'. Add to it. */
                if (kstr_equal_cstr(prev, "..")) {
                    karray_push(&stack, prev);
                    karray_push(&stack, s);
                }
                
                /* Discard the previous component and the '..'. */
                else {
                    kstr_destroy(prev);
                    kstr_destroy(s);
                }
            }
        }
        
        /* Ignore '.'. */
        else if (kstr_equal_cstr(s, ".")) {
            kstr_destroy(s);
        }
        
        /* Add the component. */
        else {
            karray_push(&stack, s);
        }
    }
    
    /* Clear the current component array. */
    karray_reset(&dir->components);
    
    /* Transfer the components. */
    for (i = 0; i < stack.size; i++) karray_push(&dir->components, stack.data[i]);
    
    karray_clean(&stack);
}

/* This function normalizes a path. It simplifies the directory portion and
 * makes the path absolute if requested.
 */
void kpath_normalize(kstr *path, int absolute_flag, int format) {
    struct kpath_dir dir;
    kstr dir_path, filename, ext;
    
    kstr_init(&dir_path);
    kstr_init(&filename);
    kstr_init(&ext);
    kpath_dir_init(&dir);
    
    /* If requested, make the path absolute. */
    if (absolute_flag) kpath_make_absolute(path, format);
    
    /* Split the path. */
    kpath_split(path, &dir_path, &filename, &ext, format);
    
    /* Decompose, simplify and recompose the directory portion. */
    kpath_decompose_dir(&dir_path, &dir, format);
    kpath_simplify_dir(&dir);
    kpath_recompose_dir(&dir_path, &dir, format);
    
    /* Recompose the path. */
    kstr_assign_kstr(path, &dir_path);
    kstr_append_kstr(path, &filename);
    
    if (ext.slen) {
        kstr_append_char(path, '.');
        kstr_append_kstr(path, &ext);
    }
    
    kpath_dir_clean(&dir);
    kstr_clean(&dir_path);
    kstr_clean(&filename);
    kstr_clean(&ext);
}
    
/* This function retrieves the current working directory. The path will have a
 * trailing delimiter.
 */
void kpath_getcwd(kstr *path) {

    #ifdef __WINDOWS__
    #define getcwd _getcwd
    #endif

    int buf_size = 256;
    char *ret;
    kbuffer buf;
    
    kbuffer_init(&buf);

    while (1) {
        kbuffer_grow(&buf, buf_size);
        ret = getcwd((char *) buf.data, buf_size);

        if (ret == NULL) {
            
            /* Buffer is too small. Double it. */
            if (errno == ERANGE) buf_size *= 2;			

            /* Oops. */
            else kerror_fatal("cannot get current working directory: %s", kerror_syserror());
        }

        else break;
    }
    
    kstr_assign_buf(path, buf.data, strlen((char *) buf.data));
    kpath_add_delim(path, KPATH_FORMAT_NATIVE); 
    
    kbuffer_clean(&buf);
}

