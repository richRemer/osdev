#pragma once

#include <stdbool.h>
#include <fuse3/fuse.h>

enum dbfs_opt {
    OPT_FILE,
    OPT_HELP,
};

struct dbfs_config {
    bool show_help;
    const char* db_file;
    const char* mnt_dir;
};

bool dbfs_getopt(struct fuse_args*, struct dbfs_config*);
