#define FUSE_USE_VERSION 38

#include "getopt.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static struct fuse_opt options[] = {
    FUSE_OPT_KEY("-h", OPT_HELP),
    FUSE_OPT_KEY("--help", OPT_HELP),
    FUSE_OPT_KEY("-f %s", OPT_FILE),
    FUSE_OPT_END
};

static int process(void* data, const char* arg, int key, struct fuse_args* args) {
    struct dbfs_config* config = data;

    switch (key) {
        case OPT_HELP:
            config->show_help = true;
            break;
        case OPT_FILE:
            config->db_file = strdup(arg+2);
            break;
        case FUSE_OPT_KEY_OPT:
            fprintf(stderr, "unknown option -- %s\n", arg);
            exit(1);
            break;
        case FUSE_OPT_KEY_NONOPT:
            if (config->mnt_dir == NULL) {
                config->mnt_dir = strdup(arg);
            } else {
                fprintf(stderr, "unknown argument -- %s\n", arg);
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "unknown option key -- %d for %s\n", key, arg);
            exit(1);
            break;
    }

    return 1;
}

bool dbfs_getopt(struct fuse_args* args, struct dbfs_config* config) {
    memset(config, 0, sizeof(struct dbfs_config));
    return 0 == fuse_opt_parse(args, config, options, process);
}

