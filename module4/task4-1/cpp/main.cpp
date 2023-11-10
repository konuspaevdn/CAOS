#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

extern "C" {
#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FUSE_USE_VERSION 30
#include <fuse.h>
}

std::vector<std::string> union_dirs;

typedef struct {
    char* src;
} options_t;

std::unordered_set<std::string>
get_subfiles(const std::string& path, int not_only_dirs = 1)
{
    std::unordered_set<std::string> files;
    DIR* dir_stream;
    int fd_dir;
    struct dirent* entry;
    struct stat st {
    };
    for (auto& d : union_dirs) {
        std::string dir = d + path;
        if (nullptr == (dir_stream = opendir(dir.c_str()))) {
            continue;
        }
        fd_dir = open(dir.c_str(), O_RDONLY);
        while (nullptr != (entry = readdir(dir_stream))) {
            fstatat(fd_dir, entry->d_name, &st, 0);
            if (not_only_dirs || S_ISDIR(st.st_mode)) {
                files.insert(std::string(entry->d_name));
            }
        }
        closedir(dir_stream);
        close(fd_dir);
    }
    if (!not_only_dirs) {
        files.erase(".");
        files.erase("..");
    }
    return files;
}

int get_latest_version(
    const char* path,
    struct stat* st,
    std::string* name = nullptr)
{
    time_t max_modify_time = -1;
    int fd_dir;
    struct stat st_buf {
    };
    for (auto& dir : union_dirs) {
        fd_dir = open(dir.c_str(), O_RDONLY);
        if (-1 == fstatat(fd_dir, path + 1, &st_buf, 0)) {
            close(fd_dir);
            continue;
        }
        if (st_buf.st_mtim.tv_sec > max_modify_time) {
            max_modify_time = (time_t)st_buf.st_mtim.tv_sec;
            if (S_ISREG(st_buf.st_mode)) {
                st->st_mode = 0444 | S_IFREG;
                st->st_nlink = st_buf.st_nlink;
                st->st_size = st_buf.st_size;
            } else if (S_ISDIR(st_buf.st_mode)) {
                st->st_mode = 0555 | S_IFDIR;
                st->st_nlink = 2 + get_subfiles(path, 0).size();
            }
            if (name) {
                *name = dir + path;
            }
        }
        close(fd_dir);
    }
    return -1 == max_modify_time;
}

int callback_stat(const char* path, struct stat* st, struct fuse_file_info* fi)
{
    if (strcmp("/", path) == 0) {
        st->st_mode = 0555 | S_IFDIR;
        st->st_nlink = 2 + get_subfiles("/", 0).size();
        return 0;
    }
    if (get_latest_version(path, st) != 0) {
        return -ENOENT;
    }
    return 0;
}

int callback_readdir(
    const char* path,
    void* out,
    fuse_fill_dir_t filler,
    off_t off,
    struct fuse_file_info* fi,
    enum fuse_readdir_flags flags)
{
    if (strcmp("/", path) == 0) {
        for (auto& dir : get_subfiles("/")) {
            filler(out, dir.c_str(), nullptr, 0, (enum fuse_fill_dir_flags)0);
        }
    } else {
        struct stat st {
        };
        if (get_latest_version(path, &st) != 0) {
            return -ENOENT;
        }
        if (S_ISDIR(st.st_mode)) {
            for (auto& dir : get_subfiles(path)) {
                filler(
                    out, dir.c_str(), nullptr, 0, (enum fuse_fill_dir_flags)0);
            }
        } else {
            filler(out, path, nullptr, 0, (enum fuse_fill_dir_flags)0);
        }
    }
    return 0;
}

int callback_open(const char* path, struct fuse_file_info* fi)
{
    if (strcmp("/", path) == 0) {
        return -EISDIR;
    }
    struct stat st {
    };
    std::string name;
    if (get_latest_version(path, &st, &name) != 0) {
        return -ENOENT;
    }
    if (-1 == (fi->fh = open(name.c_str(), fi->flags))) {
        return -errno;
    }
    return 0;
}

int callback_read(
    const char* path,
    char* buf,
    size_t buf_size,
    off_t offset,
    struct fuse_file_info* fi)
{
    if (-1 == lseek((int)fi->fh, offset, SEEK_SET)) {
        return -errno;
    }
    ssize_t bytes_read;
    if (-1 == (bytes_read = read((int)fi->fh, buf, buf_size))) {
        return -errno;
    }
    return (int)bytes_read;
}

int main(int argc, char* argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    options_t options;
    memset(&options, 0, sizeof(options));

    struct fuse_opt opt_specs[] = {
        {"--src %s", offsetof(options_t, src), 0}, {nullptr, 0, 0}};

    fuse_opt_parse(&args, &options, opt_specs, nullptr);

    std::string dirs_list;

    if (options.src) {
        dirs_list = options.src;
    } else {
        std::cerr << "No directories to union\n";
        return 1;
    }

    // checking if given files are accessible directories
    std::string directory_name;
    directory_name.reserve(PATH_MAX);
    struct stat st {
    };
    size_t str_length = dirs_list.size();
    for (size_t i = 0; i < str_length + 1; ++i) {
        if (':' == dirs_list[i] || i == str_length) {
            if (stat(directory_name.c_str(), &st) != 0) {
                std::cerr << directory_name << " is not accessible\n";
                return 1;
            }
            if (!S_ISDIR(st.st_mode)) {
                std::cerr << directory_name << " is not a directory\n";
                return 1;
            }
            union_dirs.emplace_back(realpath(directory_name.c_str(), nullptr));
            directory_name.clear();
        } else {
            directory_name.push_back(dirs_list[i]);
        }
    }

    struct fuse_operations handlers = {
        .getattr = callback_stat,
        .open = callback_open,
        .read = callback_read,
        .readdir = callback_readdir,
    };

    int ret = fuse_main(args.argc, args.argv, &handlers, nullptr);
    fuse_opt_free_args(&args);

    return ret;
}