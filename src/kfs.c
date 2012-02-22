/**
 * src/kfs.c
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "kfs.h"
#include "kerror.h"
#include "karray.h"

/* This function opens a file in mode 'mode' ('mode' is the standard argument of
 * fopen(). Better to add the 'b' flag in case we run on Windows).
 * This function sets the KMO error string. It returns -1 on failure.
 * Arguments:
 * Handle to the file pointer that will be set.
 * Path to the file.
 * Mode of the file.
 */
int kfs_fopen(FILE **file_handle, char *path, char *mode) {
    #ifdef __UNIX__
    FILE *file = fopen64(path, mode);
    #else
    FILE *file = fopen(path, mode);
    #endif

    if (file == NULL) {
    	KTOOLS_ERROR_SET("cannot open %s: %s", path, kerror_syserror());
    	return -1;
    }

    *file_handle = file;
    return 0;
}

/* This function closes a file. file_handle will be set to NULL even on error.
 * The function does nothing if *file_handle is NULL.
 * This function sets the KMO error string. It returns -1 on failure.
 * Arguments:
 * Handle to the pointer of the file to close.
 * Silence error flag (do not set the error string).
 */
int kfs_fclose(FILE **file_handle, int silent_flag) {
    int error = 0;
    
    if (*file_handle == NULL)
    	return 0;

    error = fclose(*file_handle);
    *file_handle = NULL;

    if (error && ! silent_flag) {
    	KTOOLS_ERROR_SET("cannot close file: %s", kerror_syserror());
	return -1;
    }
    
    return 0;
}

/* This function truncates a file to 0 byte and sets the file position to 0.
 * This function sets the KMO error string. It returns -1 on failure.
 * Arguments:
 * File descriptor of the file.
 */
int kfs_ftruncate(FILE *file) {
    #ifdef __UNIX__
    if (ftruncate(fileno(file), 0)) {
    #endif

    #ifdef __WINDOWS__
    if (_chsize(fileno(file), 0)) {
    #endif
	KTOOLS_ERROR_SET("cannot truncate file: %s", kerror_syserror());
    	return -1;
    }
    
    if (kfs_fseek(file, 0, SEEK_SET)) {
    	return -1;
    }
    
    return 0;
}

/* This function reads the number of bytes specified.
 * This function sets the KMO error string. It returns -1 on failure.
 * Arguments:
 * File pointer.
 * Buffer pointer.
 * Buffer size.
 */
int kfs_fread(FILE *file, void *buf, size_t size) {

    /* Note: do not use 'fread(buf, size, 1, file) != 1', because it does
     * not work when the user tries to read 0 byte.
     */
    if (fread(buf, 1, size, file) != size) {
    
	/* Error occurred because of end of file. */
	if (feof(file)) {
	    KTOOLS_ERROR_SET("cannot read data from file: end of file reached");
	    return -1;
	}

	/* Other error. */
	KTOOLS_ERROR_SET("cannot read data from file: %s", kerror_syserror());
	return -1;
    }
    
    return 0;
}

/* Same as above, but the data is written. */
int kfs_fwrite(FILE *file, void *buf, size_t size) {
    if (fwrite(buf, 1, size, file) != size) {
	KTOOLS_ERROR_SET("cannot write data to file: %s", kerror_syserror());
	return -1;
    }
    
    return 0;
}

/* Read the content of the file specified in the buffer specified. The maximum
 * file size is 1GB.
 */
int kfs_read_file(char *path, kbuffer *buf) {
    int error = 0;
    uint64_t size;
    FILE *file = NULL;
    
    kbuffer_reset(buf);
    
    do {
        if (kfs_fopen(&file, path, "rb") || kfs_fsize(file, &size)) {
            error = -1;
            break;
        }
        
        if (size > 1024*1024*1024) {
            KTOOLS_ERROR_SET("file too large");
            error = -1;
            break;
        }
        
        if (kfs_fread(file, kbuffer_write_nbytes(buf, size), size) || kfs_fclose(&file, 0)) {
            error = -1;
            break;
        }
        
    } while (0);
        
    kfs_fclose(&file, 1);
    return error;
}

/* Write the content of the buffer specified in the file specified. */
int kfs_write_file(char *path, kbuffer *buf) {
    int error = 0;
    FILE *file = NULL;
    
    if (kfs_fopen(&file, path, "wb") ||
        kfs_fwrite(file, buf->data, buf->len) ||
        kfs_fclose(&file, 0)) {
        error = -1;
    }
    
    kfs_fclose(&file, 1);
    return error;
}

/* This function gets the current position in the file specified.
 * This function sets the KMO error string. It returns -1 on failure.
 * Arguments:
 * File pointer.
 * Pointer to the position to set.
 */
int kfs_ftell(FILE *file, uint64_t *pos) {
    #ifdef __UNIX__
    int64_t ipos = ftello(file);
    #else
    int64_t ipos = ftello64(file);
    #endif

    if (ipos == -1) {
	KTOOLS_ERROR_SET("cannot get current position in file: %s", kerror_syserror());
	return -1;
    }
    
    *pos = ipos;
    return 0;
}
 
/* This function gets the size of an open file. Warning: it will seek at the 
 * beginning of the file.
 * This function sets the KMO error string. It returns -1 on failure.
 * Arguments:
 * File pointer.
 * Pointer to the size to set.
 */
int kfs_fsize(FILE *file, uint64_t *size) {

    /* Seek to the end. */
    if (kfs_fseek(file, 0, SEEK_END)) {
    	return -1;
    }
    
    /* Get the current position. */
    if (kfs_ftell(file, size)) {
    	return -1;
    }

    /* Seek to the beginning. */
    rewind(file);
    
    return 0;
}

/* This function seeks in a file.
 * This function sets the KMO error string. It returns -1 on failure.
 * Arguments:
 * File ptr, offset, whence, like with the fseek() function.
 */
int kfs_fseek(FILE *file, int64_t offset, int whence) {
    
    #ifdef __UNIX__
    if (fseeko(file, offset, whence)) {
    #else
    if (fseeko64(file, offset, whence)) {
    #endif
    	KTOOLS_ERROR_SET("cannot seek in file: %s", kerror_syserror());
	return -1;
    }
    
    return 0;
}

/* Flush the file specified. */
int kfs_flush(FILE *file) {
    if (fflush(file)) {
    	KTOOLS_ERROR_SET("cannot flush file: %s", kerror_syserror());
	return -1;
    }
    
    return 0;
}

/* This function renames file 'from_path' to 'to_path'. to_path will be
 * overwritten if it exists. Make sure you don't cross filesystem boundaries.
 * This function sets the KMO error string. It returns -1 on failure.
 * Arguments:
 * Path of the file to rename.
 * Destination path.
 */
int kfs_rename(char *from_path, char *to_path) {
    if (rename(from_path, to_path) == -1) {
    	KTOOLS_ERROR_SET("failed to rename %s to %s: %s", from_path, to_path, kerror_syserror());
	return -1;
    }
    
    return 0;
}

/* This function returns true if the file specified is a regular file.
 * Arguments:
 * Path to the file to check.
 */
int kfs_regular(char *path) {
    #ifdef __UNIX__
    struct stat stat_buf;
    int error = stat(path, &stat_buf);
    return (error != -1 && S_ISREG(stat_buf.st_mode));
    #endif

    /* Does not do exactly what we want, but it'll have to do. */
    #ifdef __WINDOWS__
    unsigned int error = GetFileAttributes(path);
    return (error != INVALID_FILE_ATTRIBUTES && (! (error & FILE_ATTRIBUTE_DIRECTORY)));
    #endif
}

/* This function deletes the regular file 'path', which must exists.
 * This function sets the KMO error string. It returns -1 on failure.
 * Arguments:
 * Path to the file to delete.
 * Silence error flag (do not set the error string).
 */
int kfs_delete(char *path, int silent_flag) {
    if (remove(path) == -1 && !silent_flag) {
    	KTOOLS_ERROR_SET("failed to delete %s: %s", path, kerror_syserror());
	return -1;
    }
    
    return 0;
}

/* This function returns true if the file specified is a directory.
 * Arguments:
 * Path to the file to check.
 */
int kfs_isdir(char *path) {
    #ifdef __WINDOWS__
    unsigned int error = GetFileAttributes(path);
    return (error != INVALID_FILE_ATTRIBUTES && (error & FILE_ATTRIBUTE_DIRECTORY));
    #else
    struct stat stat_buf;
    int error = stat(path, &stat_buf);
    return (error != -1 && S_ISDIR(stat_buf.st_mode));
    #endif
}

/* This function creates the directory specified.
 * This function sets the KMO error string. It returns -1 on failure.
 * Arguments:
 * Path to the directory to create.
 */
int kfs_mkdir(char *path) {
    #ifdef __WINDOWS__
    if (_mkdir(path) == -1)
    #else
    if (mkdir(path, 0777) == -1)
    #endif
    {
    	KTOOLS_ERROR_SET("cannot create directory %s: %s", path, kerror_syserror());
	return -1;
    }
    
    return 0;
}

/* This function fills an array with the name of the files and directories
 * contained within the directory specified. The array's content must be freed
 * by the user.
 * This function sets the KMO error string. It returns -1 on failure.
 * Arguments:
 * Path to the directory to list.
 * Array that will contain the file names.
 */
int kfs_ls(char *path, karray *listing) {
    int error = 0;
    listing->size = 0;
    
    #ifdef __UNIX__
    DIR *dir = NULL;
    struct dirent *dir_entry;
    
    /* Try. */
    do {
        dir = opendir(path);

        if (dir == NULL) {
	    KTOOLS_ERROR_SET("cannot list directory %s: %s", path, kerror_syserror());
	    error = -1;
	    break;
	}

        /* Loop until there are no more files. */
        while (1) {
            dir_entry = readdir(dir);
	    kstr *file_name = NULL;
    	    
	    /* OK, error occurred because there is no file (we must assume this
             * since readdir()'s semantics are not fully specified).
	     */
            if (dir_entry == NULL) {
        	break;
            }

            /* Skip '.' and '..'. */
            if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) {
        	continue;
            }

            /* Add the file name in the array. */
	    file_name = kstr_new();
	    kstr_assign_cstr(file_name, dir_entry->d_name);
            karray_push(listing, file_name);
        }
	
    } while (0);
    
    if (dir) closedir(dir);
    #endif

    /* A twisted mind you need indeed to come up with functions such as
     * FindFirstFile() and FindNextFile().
     */
    #ifdef __WINDOWS__
    HANDLE search_handle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA find_data;
    int first_flag = 1;
    kstr *file_name;

    /* Try. */
    do {
        /* Loop until there are no more files. */
        while (1) {
	
            /* Get the data about the next file, if any. */
            if (first_flag) {
                first_flag = 0;
                search_handle = FindFirstFile(path, &find_data);

                if (search_handle == INVALID_HANDLE_VALUE) {
		    error = GetLastError();
		    
                    if (error != ERROR_FILE_NOT_FOUND && error != ERROR_NO_MORE_FILES) {
                        KTOOLS_ERROR_SET("cannot list directory %s", path);
			error = -1;
			break;
                    }
		    
		    else {
		    	error = 0;
		    }

                    /* OK, error occurred because there was no file. */
                    break;
                }
            }

            else {
                if (FindNextFile(search_handle, &find_data) == 0) {
                    error = GetLastError();
		    
		    if (error != ERROR_FILE_NOT_FOUND && error != ERROR_NO_MORE_FILES) {
                        KTOOLS_ERROR_SET("cannot list directory %s", path);
			error = -1;
			break;
                    }
		    
		    else {
		    	error = 0;
		    }

                    /* OK, error occurred because there was no file. */
                    break;
                }
            }

            /* Add the file name in the array. */
	    file_name = kstr_new();
	    kstr_assign_cstr(file_name, find_data.cFileName);
            karray_push(listing, file_name);
        }
	
	if (error) break;
	
    } while (0);

    if (search_handle != INVALID_HANDLE_VALUE) FindClose(search_handle);
    #endif
    
    if (error) karray_clear_kstr(listing);
    
    return error;
}
