#define FUSE_USE_VERSION 38

#include <fuse3/fuse.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "getopt.h"
#include "fs.h"

static struct fuse_operations ops = {
    .getattr = dbfs_getattr,
    .open = dbfs_open,
    .read = dbfs_read,
    .readdir = dbfs_readdir,
};

int main(int argc, char* argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct dbfs_config config;

    if (!dbfs_getopt(&args, &config)) {
        fuse_opt_free_args(&args);
        fprintf(stderr, "couldn't read options\n");
        return EXIT_FAILURE;
    }

    if (config.show_help) {
        printf("Usage: dbfs -f fs.db /mnt\n");
        return EXIT_SUCCESS;
    }

    fuse_opt_insert_arg(&args, 1, config.mnt_dir);

    args.argc = 2;
    args.argv[2] = NULL;
    
    int result = fuse_main(args.argc, args.argv, &ops, NULL);
    
    fuse_opt_free_args(&args);

    return result;
}
