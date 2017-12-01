/*
 * Copyright (c) 2017-present Orlando Bassotto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "system_fuse.h"

#include "volume.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define from_object(obj) \
    reinterpret_cast<uintptr_t>(obj)
#define to_object(ffi) \
    reinterpret_cast<apfs_fuse::object *>(static_cast<uintptr_t>((ffi)->fh))
#define to_file(ffi) \
    reinterpret_cast<apfs_fuse::file *>(static_cast<uintptr_t>((ffi)->fh))
#define to_directory(ffi) \
    reinterpret_cast<apfs_fuse::directory *>(static_cast<uintptr_t>((ffi)->fh))

apfs_fuse::volume *apfs_fuse::the_volume = nullptr;

#if FUSE_VERSION_LT(2, 9)
static int
apfs_statfs(char const *path, struct statvfs *st)
{
    apfs_fuse::the_volume->stat(st);
    return 0;
}
#else
static int
apfs_statfs(char const *path, apfs_fuse::statfs_t *st)
{
    apfs_fuse::the_volume->stat(st);
    return 0;
}
#endif

static int
apfs_getattr(char const *path, struct stat *st)
{
    errno = 0;
    auto o = apfs_fuse::the_volume->open(path);
    if (o == nullptr)
        return -errno;

    errno = 0;
    int rc = o->getattr(st);
    delete o;

    if (rc < 0)
        return -errno;

    return rc;
}

static int
apfs_listxattr(char const *path, char *namebuf, size_t size)
{
    errno = 0;
    auto o = apfs_fuse::the_volume->open(path);
    if (o == nullptr)
        return -errno;

    errno = 0;
    int rc = o->listxattr(namebuf, size);
    delete o;

    if (rc < 0)
        return -errno;

    return rc;
}

static int
apfs_getxattr(char const *path, char const *name, char *buf, size_t size
        POSITION_ARG)
{
    POSITION_DECL;

    errno = 0;
    auto o = apfs_fuse::the_volume->open(path);
    if (o == nullptr)
        return -errno;

    errno = 0;
    ssize_t rc = o->getxattr(name, buf, size, position);
    delete o;

    if (rc < 0)
        return -errno;

    return rc;
}

static int
apfs_readlink(char const *path, char *buf, size_t bufsize)
{
    errno = 0;
    auto o = apfs_fuse::the_volume->open(path);
    if (o == nullptr)
        return -errno;

    errno = 0;
    ssize_t rc = o->readlink(buf, bufsize);
    delete o;

    if (rc < 0)
        return -errno;

    return rc;
}

static int
apfs_open(char const *path, struct fuse_file_info *ffi)
{
    errno = 0;
    auto f = apfs_fuse::the_volume->open_file(path);
    if (f == nullptr)
        return -errno;

    ffi->fh = from_object(f);
    return 0;
}

static int
apfs_release(char const *path, struct fuse_file_info *ffi)
{
    auto f = to_file(ffi);
    if (f == nullptr)
        return -EBADF;
    if (!f->is_regular())
        return f->is_directory() ? -EISDIR : -EINVAL;

    delete f;
    ffi->fh = 0;

    return 0;
}

static int
apfs_read(char const *path, char *buf, size_t size, off_t offset,
        struct fuse_file_info *ffi)
{
    auto f = to_file(ffi);

    if (f == nullptr)
        return -EBADF;
    if (!f->is_regular())
        return f->is_directory() ? -EISDIR : -EINVAL;

    errno = 0;
    ssize_t nread = f->read(buf, size, offset);
    if (nread < 0)
        return -errno;

    return nread;
}

static int
apfs_opendir(char const *path, struct fuse_file_info *ffi)
{
    errno = 0;
    auto d = apfs_fuse::the_volume->open_directory(path);
    if (d == nullptr)
        return -errno;

#if !FUSE_VERSION_LT(2, 8)
    ffi->nonseekable = 1;
#endif
    ffi->fh = from_object(d);

    return 0;
}

static int
apfs_readdir(char const *path, void *dirbuf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *ffi)
{
    auto d = to_directory(ffi);
    if (d == nullptr)
        return -EBADF;
    if (!d->is_directory())
        return -ENOTDIR;

    if (offset == 0) {
        d->rewind();
    }

    std::string name;
    uint64_t file_id;
    off_t next_offset;

    while (d->next(offset, name, file_id, next_offset)) {
        struct stat st;
        struct stat *stp = nullptr;

        //
        // If exposing resource fork or xattr directory,
        // then we must fake inodes.
        //
        if (!d->is_virtual() && !apfs_fuse::expose_resource_fork &&
                !apfs_fuse::expose_xattr_directory) {
            memset(&st, 0, sizeof(st));
            st.st_ino = file_id;

            stp = &st;
        }

        if (filler(dirbuf, name.c_str(), stp, next_offset) != 0)
            return -EIO;

        if (apfs_fuse::expose_resource_fork) {
            //
            // Create a fake entry to the resource fork
            //
        }

#ifdef FUSE_READDIR_SINGLE_READ
        offset = next_offset;
#else
        break;
#endif
    }

    return 0;
}

static int
apfs_releasedir(char const *path, struct fuse_file_info *ffi)
{
    auto d = to_directory(ffi);
    if (d == nullptr)
        return -EBADF;
    if (!d->is_directory())
        return -ENOTDIR;

    delete d;
    ffi->fh = 0;

    return 0;
}

static int
apfs_fgetattr(const char *path, struct stat *st, struct fuse_file_info *ffi)
{
    auto o = to_object(ffi);
    if (o == nullptr)
        return -EBADF;

    errno = 0;
    int rc = o->getattr(st);
    if (rc < 0)
        return -errno;

    return rc;
}

namespace {

struct fuse_operations fuse_ops;

struct init_fuse_ops {
    init_fuse_ops()
    {
        fuse_ops.getattr    = apfs_getattr;
        fuse_ops.readlink   = apfs_readlink;
        fuse_ops.open       = apfs_open;
        fuse_ops.read       = apfs_read;
#if !defined(__APPLE__) || FUSE_VERSION_LT(2, 9)
        fuse_ops.statfs     = apfs_statfs;
#endif
        fuse_ops.release    = apfs_release;
        fuse_ops.getxattr   = apfs_getxattr;
        fuse_ops.listxattr  = apfs_listxattr;
        fuse_ops.opendir    = apfs_opendir;
        fuse_ops.readdir    = apfs_readdir;
        fuse_ops.releasedir = apfs_releasedir;
        fuse_ops.fgetattr   = apfs_fgetattr;
#if !FUSE_VERSION_LT(2, 9)
        // fuse_ops.read_buf   = apfs_read_buf;
#endif
#if defined(__APPLE__) && !FUSE_VERSION_LT(2, 9)
        fuse_ops.statfs_x   = apfs_statfs;
#endif
    }
} __fuse_ops_initializer;

}

#ifdef __APPLE__
namespace {

static char *volume_icon_path = nullptr;

static void
remove_volume_icon()
{
    if (volume_icon_path != nullptr) {
        unlink(volume_icon_path);
    }
}

static int
create_volume_icon(char const *progname)
{
    int fd = -1;
    char buf[128];

    auto uuid = apfs_fuse::the_volume->get_uuid();
    nx_uuid_format(&uuid, buf, sizeof(buf));

    if (asprintf(&volume_icon_path, "/tmp/%s-%s.icns", progname, buf) < 0)
        goto fail;

    fd = open(volume_icon_path, O_WRONLY|O_CREAT, 0644);
    if (fd < 0)
        goto fail;

    fprintf(stderr, "info: extracted volume icon to '%s'\n", volume_icon_path);
    atexit(remove_volume_icon);

    return fd;

fail:
    fprintf(stderr, "warning: cannot create volume icon: %s\n",
            strerror(errno));
    return -1;
}

}
#endif

char const *apfs_fuse::
extract_volume_icon(char const *progname)
{
#ifdef __APPLE__
    auto f = the_volume->open_file("/.VolumeIcon.icns");
    if (f != nullptr) {
        size_t size = f->get_size();
        int fd = create_volume_icon(progname);
        if (fd >= 0) {
            char buf[1024];
            ssize_t nread;
            off_t offset = 0;
            while ((nread = f->read(buf, sizeof(buf), offset)) != 0) {
                write(fd, buf, nread);
                offset += nread;
            }
            close(fd);
        }
        delete f;
        return volume_icon_path;
    }

    return nullptr;
#endif
}

int apfs_fuse::
main(std::vector<char const *> const &args)
{
    return fuse_main(args.size(), const_cast<char **>(&args[0]),
            &fuse_ops, the_volume);
}
