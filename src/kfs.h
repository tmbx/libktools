/**
 * src/kfs.c
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 */

#ifndef __K_FS_H__
#define __K_FS_H__

#include <stdio.h>
#include <karray.h>

int kfs_fopen(FILE **file_handle, char *path, char *mode);
int kfs_fclose(FILE **file_handle, int silent_flag);
int kfs_ftruncate(FILE *file);
int kfs_fread(FILE *file, void *buf, size_t size);
int kfs_fwrite(FILE *file, void *buf, size_t size);
int kfs_read_file(char *path, kbuffer *buf);
int kfs_write_file(char *path, kbuffer *buf);
int kfs_ftell(FILE *file, uint64_t *pos);
int kfs_fsize(FILE *file, uint64_t *size);
int kfs_fseek(FILE *file, int64_t offset, int whence);
int kfs_flush(FILE *file);
int kfs_rename(char *from_path, char *to_path);
int kfs_regular(char *path);
int kfs_delete(char *path, int silent_flag);
int kfs_isdir(char *path);
int kfs_mkdir(char *path);
int kfs_ls(char *path, karray *listing);

#endif /*__K_FS_H__*/
