#define FUSE_USE_VERSION 38

#include <sys/stat.h>
#include <fuse3/fuse.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>

static const char* metadata = "{\"name\":\"foo\"}\n";

static int dbfs_getattr(
    const char* path,
    struct stat* stat,
    struct fuse_file_info* info
) {
    memset(stat, 0, sizeof(struct stat));

    if (0 == strcmp(path, "/")) {
        stat->st_ino = 1;
        stat->st_mode = S_IFDIR | 0755;
        stat->st_nlink = 2;
        stat->st_uid = 0;
        stat->st_gid = 0;
        stat->st_size = 0;
    } else if (0 == strcmp(path, "/.meta")) {
        const size_t size = strlen(metadata);
        stat->st_ino = 2;
        stat->st_mode = S_IFREG | 0644;
        stat->st_nlink = 1;
        stat->st_uid = 0;
        stat->st_gid = 0;
        stat->st_size = size;
    } else {
        return -ENOENT;
    }

    stat->st_dev = 42;

    return 0;
}

static int dbfs_open(const char* path, struct fuse_file_info* info) {
    return 0;
}

static int dbfs_read(
    const char* path,
    char* buf,
    size_t size,
    off_t offset,
    struct fuse_file_info* info
) {
    if (0 == strcmp(path, "/.meta")) {
        const size_t len = strlen(metadata);
        
        if (offset >= len) {
            return 0;
        }

        if (offset + size > len) {
            memcpy(buf, metadata + offset, len - offset);
            return len - offset;
        }

        memcpy(buf, metadata + offset, size);
        return size;
    }

    return -ENOENT;
}

static int dbfs_readdir(
    const char* path,
    void* buf,
    fuse_fill_dir_t filler,
    off_t offset,
    struct fuse_file_info* info,
    enum fuse_readdir_flags flags
) {
    (void)offset;
    (void)info;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, ".meta", NULL, 0, 0);

    return 0;
}

static struct fuse_operations ops = {
    .getattr = dbfs_getattr,
    .open = dbfs_open,
    .read = dbfs_read,
    .readdir = dbfs_readdir,
};

int main(int argc, char* argv[]) {
    return fuse_main(argc, argv, &ops, NULL);
}
