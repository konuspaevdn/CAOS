#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define FUSE_USE_VERSION 30
#include <fuse.h>

/* (hard coding) Directory structure:
/
- a.txt
- b.txt
*/

static const char FileA[] ="Hello, world!\n";
static const char FileB[] ="abrakadabra\n";

int my_readdir(const char *path, void *out, fuse_fill_dir_t filler,
            off_t off, struct fuse_file_info *fi,
                    enum fuse_readdir_flags flags) {
    if (strcmp("/", path) != 0) {
        // path != "/"
        return -ENOENT;
    }
    filler(out, ".", NULL, 0, 0);
    filler(out, "..", NULL, 0, 0);
    filler(out, "a.txt", NULL, 0, 0);
    filler(out, "b.txt", NULL, 0, 0);
    return 0;
}

int my_stat(const char *path, struct stat *st, struct fuse_file_info *fi) {
    if (strcmp("/", path) == 0) {
        st->st_mode = 0550 | S_IFDIR;
        st->st_nlink = 2;
        return 0;
    }
    if (strcmp("/a.txt", path) == 0) {
        st->st_mode = 0440 | S_IFREG;
        st->st_nlink = 1;
        st->st_size = strlen(FileA);
        return 0;
    }
    if (strcmp("/b.txt", path) == 0) {
        st->st_mode = 0440 | S_IFREG;
        st->st_nlink = 1;
        st->st_size = strlen(FileB);
        return 0;
    }
    return -ENOENT;
}

int my_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp("/a.txt", path) != 0 && strcmp("/b.txt", path) != 0) {
        return -ENOENT;
    }
    return 0;
}

int my_read(const char *path, char *buffer, size_t buf_size, off_t offset,
            struct fuse_file_info *fi) {
    // returns bytes read
    char *data = NULL;
    if (strcmp("/a.txt", path) == 0) {
        data = FileA;
    }
    if (strcmp("/b.txt", path) == 0) {
        data = FileB;
    }
    if (!data) {
        return -ENOENT;
    }

    size_t file_size = strlen(data);
    size_t size = 0;
    if (buf_size > file_size - offset) {
        size = file_size - offset;
    } else {
        size = buf_size;
    }
    if (size > 0) {
        memcpy(buffer, data, size);
    }
    return size;
}

int main(int argc, char* argv[]) {
    struct fuse_operations handlers = {
            .readdir = my_readdir,
            .getattr = my_stat,
            .open = my_open,
            .read = my_read
    };
    return fuse_main(argc, argv, &handlers, NULL);
}