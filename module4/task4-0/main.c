#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FUSE_USE_VERSION 30
#include <fuse.h>

typedef struct {
    char* src;
} options_t;

int fd;

void read_number(size_t* number)
{
    char buffer;
    char integer[32] = {};
    size_t iter = 0;
    while (1) {
        read(fd, &buffer, sizeof(buffer));
        if ('\n' == buffer) {
            break;
        }
        integer[iter++] = buffer;
    }
    *number = strtoll(integer, NULL, 10);
}

void read_filename(char* filename)
{
    char buffer;
    size_t iter = 0;
    while (1) {
        read(fd, &buffer, sizeof(buffer));
        if (' ' == buffer) {
            break;
        }
        filename[iter++] = buffer;
    }
}

void skip_line_remnants()
{
    char buffer;
    while (1) {
        read(fd, &buffer, sizeof(buffer));
        if ('\n' == buffer) {
            break;
        }
    }
}

int callback_stat(const char* path, struct stat* st, struct fuse_file_info* fi)
{
    if (strcmp("/", path) == 0) {
        st->st_mode = 0555 | S_IFDIR;
        st->st_nlink = 2;
        return 0;
    }

    lseek(fd, 0, SEEK_SET);

    size_t count;
    read_number(&count);

    char filename[NAME_MAX];
    for (size_t i = 0; i < count; ++i) {
        memset(filename, 0, sizeof(filename));
        read_filename(filename);
        if (strcmp(filename, path + 1) == 0) {
            // file is found - provide mode, hardlinks and size statistics
            size_t size;
            read_number(&size);
            st->st_size = (off_t)size;
            st->st_mode = 0444 | S_IFREG;
            st->st_nlink = 1;
            return 0;
        } else {
            skip_line_remnants();
        }
    }
    return -ENOENT;
}

int callback_readdir(
    const char* path,
    void* out,
    fuse_fill_dir_t filler,
    off_t off,
    struct fuse_file_info* fi,
    enum fuse_readdir_flags flags)
{
    if (strcmp("/", path) != 0) {
        // path != "/", only one dir
        return -ENOENT;
    }

    filler(out, ".", NULL, 0, 0);
    filler(out, "..", NULL, 0, 0);

    lseek(fd, 0, SEEK_SET);

    size_t count;
    read_number(&count);

    // pass all filenames
    char filename[NAME_MAX];
    for (size_t i = 0; i < count; ++i) {
        memset(filename, 0, sizeof(filename));
        read_filename(filename);
        filler(out, filename, NULL, 0, 0);
        skip_line_remnants();
    }
    return 0;
}

int callback_open(const char* path, struct fuse_file_info* fi)
{
    lseek(fd, 0, SEEK_SET);

    size_t count;
    read_number(&count);

    // check if file exists
    char filename[NAME_MAX];
    for (size_t i = 0; i < count; ++i) {
        memset(filename, 0, sizeof(filename));
        read_filename(filename);
        if (strcmp(filename, path + 1) == 0) {
            return 0;
        }
        skip_line_remnants();
    }
    return -ENOENT;
}

int callback_read(
    const char* path,
    char* buf,
    size_t buf_size,
    off_t offset,
    struct fuse_file_info* fi)
{
    lseek(fd, 0, SEEK_SET);

    size_t count;
    read_number(&count);

    // check if file exists and get its size
    int found = 0;
    char filename[NAME_MAX];
    size_t file_size;
    off_t data_offset = 0;
    for (size_t i = 0; i < count; ++i) {
        if (found) {
            skip_line_remnants();
            continue;
        }
        memset(filename, 0, sizeof(filename));
        read_filename(filename);
        if (strcmp(filename, path + 1) == 0) {
            found = 1;
            read_number(&file_size);
        } else {
            size_t add_size;
            read_number(&add_size);
            data_offset += (off_t)add_size;
        }
    }

    char buffer;
    read(fd, &buffer, sizeof(buffer));
    if ('\n' != buffer) {
        // expected an empty line
    }
    // pass unnecessary data
    lseek(fd, data_offset, SEEK_CUR);
    // read data in file
    size_t read_bytes;
    if (file_size - offset <= buf_size) {
        read_bytes = file_size - offset;
    } else {
        read_bytes = buf_size;
    }
    if (0 < read_bytes) {
        read(fd, buf, read_bytes);
    }
    return (int)read_bytes;
}

int main(int argc, char* argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    options_t my_options;
    memset(&my_options, 0, sizeof(my_options));

    struct fuse_opt opt_specs[] = {
        {"--src %s", offsetof(options_t, src), 0}, {NULL, 0, 0}};

    fuse_opt_parse(&args, &my_options, opt_specs, NULL);

    if (my_options.src) {
        char* path_to_src = my_options.src;
        if (-1 == (fd = open(path_to_src, O_RDONLY))) {
            perror("Can't open the given image file");
            return errno;
        }
    } else {
        fprintf(stderr, "No image given\n");
        return 1;
    }

    struct fuse_operations handlers = {
        .getattr = callback_stat,
        .readdir = callback_readdir,
        .open = callback_open,
        .read = callback_read};

    int ret = fuse_main(args.argc, args.argv, &handlers, NULL);
    close(fd);
    fuse_opt_free_args(&args);

    return ret;
}
