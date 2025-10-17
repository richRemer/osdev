#pragma once

#define FUSE_USE_VERSION 38

#include <stddef.h>
#include <sys/stat.h>
#include <fuse3/fuse.h>

int dbfs_getattr(const char*, struct stat*, struct fuse_file_info*);
int dbfs_open(const char*, struct fuse_file_info*);
int dbfs_read(const char*, char*, size_t, off_t, struct fuse_file_info*);
int dbfs_readdir(const char*, void*, fuse_fill_dir_t, off_t, struct fuse_file_info*, enum fuse_readdir_flags);
