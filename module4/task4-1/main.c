#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define FUSE_USE_VERSION 30
#include <fuse.h>

char* dirs_list = NULL;

typedef struct {
    char* paths[4096];
    size_t count;
} dirs_t;

dirs_t dirs;

typedef struct {
    char* src;
} options_t;

int callback_stat(const char* path, struct stat* st, struct fuse_file_info* fi)
{
    if (strcmp("/", path) == 0) {
        st->st_mode = 0555 | S_IFDIR;
        st->st_nlink = 2;  // totally lie, but let it be so???
        return 0;
    }
    time_t modify_time = 0;
    int found = 0;
    char filename[PATH_MAX];
    for (size_t i = 0; i < dirs.count; ++i) {

    }
}

int main(int argc, char* argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    options_t options;
    memset(&options, 0, sizeof(options));

    struct fuse_opt opt_specs[] = {
            {"--src %s", offsetof(options_t, src), 0}, {NULL, 0, 0}};

    fuse_opt_parse(&args, &options, opt_specs, NULL);

    if (options.src) {
        dirs_list = options.src;
    } else {
        fprintf(stderr, "No directories to union\n");
        return 1;
    }

    // checking if given files are accessible directories
    char directory_name[PATH_MAX] = {};
    size_t idx = 0;
    struct stat st;
    size_t str_length = strlen(dirs_list);
    for (size_t i = 0; i < str_length + 1; ++i) {
        if (':' == dirs_list[i] || i == str_length) {
            if (stat(directory_name, &st) != 0) {
                fprintf(stderr, "%s is not accessible\n", directory_name);
                return 1;
            }
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "%s is not a directory\n", directory_name);
                return 1;
            }
            idx = 0;
            dirs.paths[dirs.count] = realpath(directory_name, NULL);
            ++dirs.count;
            memset(directory_name, 0, sizeof(directory_name));
        } else {
            directory_name[idx++] = dirs_list[i];
        }
    }

    struct fuse_operations handlers = {
            .getattr = callback_stat,
            //.readdir = callback_readdir,
            //.open = callback_open,
            //.read = callback_read
            };

    int ret = fuse_main(args.argc, args.argv, &handlers, NULL);
    fuse_opt_free_args(&args);

    return ret;
}