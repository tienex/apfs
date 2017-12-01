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

#include "apfs/libapfs_config.h"

#include "apfs/object.h"
#include "apfs/volume.h"

#include "nxtools/native.h"
#include "nxtools/path.h"
#include "nxtools/string.h"
#include "nxtools/time.h"

#include <sys/stat.h>
#ifdef HAVE_SYS_SYSMACROS_H
#include <sys/sysmacros.h>
#endif
#ifdef HAVE_SYS_MKDEV_H
#include <sys/mkdev.h>
#endif

#include <cerrno>

using apfs::object;

object::object()
    : _volume(nullptr)
{
}

object::~object()
{
}

bool object::
open(volume *volume, nx::enumerator *e)
{
    //
    // Collect the file information
    //
    nx::object::sized_value_type key, value;
    bool insensitive = !volume->is_case_sensitive();

    while (e->next(key, value)) {
        auto obj_id = nx::swap(*reinterpret_cast<uint64_t const *>(key.first));

        switch (APFS_OBJECT_ID_TYPE(obj_id)) {
            case APFS_OBJECT_TYPE_INODE:
                file::set_inode(value.first, value.second);
                break;

            case APFS_OBJECT_TYPE_XATTR:
                file::add_xattr(key.first, value.first);
                break;

            case APFS_OBJECT_TYPE_FILE_EXTENT:
                file::add_extent(key.first, value.first);
                break;

            case APFS_OBJECT_TYPE_DREC:
                directory::add_entry(key.first, value.first, insensitive);
                break;
        }
    }

    //
    // For each indirect xattr, collect extents.
    //
    for (auto &x : file::get_xattrs()) {
        if (x.is_content_inlined())
            continue;

        auto e = volume->get_nx_volume()->open_oid(x.get_oid());
        if (e != nullptr) {
            while (e->next(key, value)) {
                auto obj_id =
                    nx::swap(*reinterpret_cast<uint64_t const *>(key.first));

                switch (APFS_OBJECT_ID_TYPE(obj_id)) {
                    case APFS_OBJECT_TYPE_FILE_EXTENT:
                        x.add_extent(key.first, value.first);
                        break;
                }
            }
            delete e;
        }
    }

    _volume = volume;
    return true;
}

void object::
release()
{
    _volume->get_cache().release(this);
}

uint16_t object::info::
native_mode() const
{
    return nxtools::apfs_mode_to_native(mode);
}

dev_t
object::info::native_dev() const
{
#ifndef _WIN32
    return makedev(APFS_DEVICE_SPEC_MAJOR(rdev),
                   APFS_DEVICE_SPEC_MINOR(rdev));
#else
    return rdev;
#endif
}

uint32_t
object::info::native_flags() const
{
    return nxtools::apfs_bsd_flags_to_native(flags);
}

void object::
get_info(info &info) const
{
    auto inode = file::get_inode();

    info.ino      = inode.private_id;
    info.mode     = inode.mode;
    info.nlink    = inode.nchildren;
    info.uid      = inode.user_id;
    info.gid      = inode.group_id;
    info.rdev     = object::get_device_spec();
    info.size     = file::get_size();
    info.atim     = nxtools::ns_to_nx_timespec(inode.access_timestamp);
    info.mtim     = nxtools::ns_to_nx_timespec(inode.modification_timestamp);
    info.ctim     = nxtools::ns_to_nx_timespec(inode.changed_timestamp);
    info.btim     = nxtools::ns_to_nx_timespec(inode.creation_timestamp);
    info.blksize  = _volume->get_block_size();
    info.blocks   = file::get_alloced_size() / 512;
    info.flags    = inode.bsd_flags;
}

void object::
stat(struct stat *st) const
{
    info info;
    get_info(info);

    if (sizeof(ino_t) == sizeof(nx_ino_t)) {
        st->st_ino = info.ino;
    }
    st->st_mode            = info.native_mode();
    st->st_nlink           = info.nlink;
    st->st_uid             = info.uid;
    st->st_gid             = info.gid;
    st->st_rdev            = info.native_dev();
    st->st_size            = info.size;
#if defined(HAVE_STAT_ST_MTIM)
    st->st_atim            = nxtools::nx_timespec_to_timespec(info.atim);
    st->st_ctim            = nxtools::nx_timespec_to_timespec(info.ctim);
    st->st_mtim            = nxtools::nx_timespec_to_timespec(info.mtim);
#if defined(HAVE_STAT_ST_BIRTHTIM)
    st->st_birthtim        = nxtools::nx_timespec_to_timespec(info.btim);
#elif defined(HAVE_STAT___ST_BIRTHTIM)
    st->__st_birthtim      = nxtools::nx_timespec_to_timespec(info.btim);
#endif
#elif defined(HAVE_STAT_ST_MTIMESPEC)
    st->st_atimespec       = nxtools::nx_timespec_to_timespec(info.atim);
    st->st_ctimespec       = nxtools::nx_timespec_to_timespec(info.ctim);
    st->st_mtimespec       = nxtools::nx_timespec_to_timespec(info.mtim);
#if defined(HAVE_STAT_ST_BIRTHTIMESPEC)
    st->st_birthtimespec   = nxtools::nx_timespec_to_timespec(info.btim);
#endif
#else
    st->st_atime           = ino.atim.tv_sec;
    st->st_ctime           = ino.ctim.tv_sec;
    st->st_mtime           = ino.mtim.tv_sec;
#if defined(HAVE_STAT_ST_BIRTHTIME)
    st->st_birthtime       = ino.btim.tv_sec;
#elif defined(HAVE_STAT___ST_BIRTHTIME)
    st->__st_birthtime     = ino.btim.tv_sec;
#endif
#if defined(HAVE_STAT_MTIMENSEC)
    st->st_atimensec       = ino.atim.tv_nsec;
    st->st_ctimensec       = ino.ctim.tv_nsec;
    st->st_mtimensec       = ino.mtim.tv_nsec;
#if defined(HAVE_STAT_ST_BIRTHTIMENSEC)
    st->st_birthtimensec   = ino.btim.tv_nsec;
#elif defined(HAVE_STAT___ST_BIRTHTIMENSEC)
    st->__st_birthtimensec = ino.btim.tv_nsec;
#endif
#endif
#endif
#ifdef HAVE_STAT_ST_BLKSIZE
    st->st_blksize         = info.blksize;
#endif
#ifdef HAVE_STAT_ST_BLOCKS
    st->st_blocks          = info.blocks;
#endif
#ifdef HAVE_STAT_ST_FLAGS
    st->st_flags           = info.native_flags();
#endif
}

bool object::
has_xattrs(unsigned flags) const
{
    if ((flags & HAS_XATTR_ANY) == 0)
        return false;

    if (flags & HAS_XATTR_THIS) {
        if (!file::get_xattrs().empty())
            return true;
    }

    bool descendent_xattr = false;

    if (is_directory() && (flags & HAS_XATTR_DESCENDENT) != 0) {
        for (auto const &i : directory::get_entries()) {
            auto *o = _volume->open(i.second.oid);
            if (o != nullptr) {
                descendent_xattr = o->has_xattrs();
                o->release();
            }

            if (descendent_xattr)
                break;
        }
    }

    return descendent_xattr;
}

size_t object::
get_xattr_count() const
{
    size_t count = 0;

    for (auto &x : file::get_xattrs()) {
        if (is_symbolic_link() && x.get_name() == APFS_XATTR_NAME_SYMLINK)
            continue;

        count++;
    }

    return count;
}

void object::
get_xattrs(string_vector &xattrs) const
{
    xattrs.clear();
    for (auto &x : file::get_xattrs()) {
        //
        // Symbolic links are stored in an xattr, so skip it.
        //
        if (is_symbolic_link() && x.get_name() == APFS_XATTR_NAME_SYMLINK)
            continue;

#ifdef __APPLE__
        //
        // Security descriptor is not listed on macOS.
        //
        if (x.get_name() == APFS_XATTR_NAME_SECURITY)
            continue;
#endif

        xattrs.push_back(x.get_name());
    }
}

bool object::
has_xattr(std::string const &name) const
{
    //
    // Symbolic links should never be exposed.
    //
    if (is_symbolic_link() && name == APFS_XATTR_NAME_SYMLINK)
        return false;

    for (auto &x : file::get_xattrs()) {
        if (x.get_name() == name)
            return true;
    }

    errno = ENOATTR;
    return false;
}

uint64_t object::
get_xattr_size(std::string const &name) const
{
    //
    // Symbolic links should never be exposed.
    //
    if (is_symbolic_link() && name == APFS_XATTR_NAME_SYMLINK) {
        errno = ENOATTR;
        return -1;
    }

    for (auto &x : file::get_xattrs()) {
        if (x.get_name() == name)
            return x.get_size();
    }

    errno = ENOATTR;
    return -1;
}

ssize_t object::
read_xattr(std::string const &name, void *buf, size_t size,
        nx_off_t offset) const
{
    //
    // Symbolic links should never be exposed.
    //
    if (is_symbolic_link() && name == APFS_XATTR_NAME_SYMLINK) {
        errno = ENOATTR;
        return -1;
    }

    for (auto &x : file::get_xattrs()) {
        if (x.get_name() == name)
            return x.read(_volume->get_session()->get_main_device(),
                    buf, size, offset);
    }

    errno = ENOATTR;
    return -1;
}

bool object::
read_symbolic_link(std::string &target) const
{
    if (!is_symbolic_link())
        return false;

    target = file::get_symbolic_link();
    return true;
}

ssize_t object::
read(void *buf, size_t size, nx_off_t offset) const
{
    if (!is_regular()) {
        errno = is_directory() ? EISDIR : EINVAL;
        return -1;
    }

    return file::read(_volume->get_session()->get_main_device(), buf, size,
            offset);
}

object *object::
traverse(std::string const &path) const
{
    if (!is_directory()) {
        errno = ENOTDIR;
        return nullptr;
    }

    auto normalized_path = nxtools::normalize_path(path);
    if (!_volume->is_case_sensitive()) {
        normalized_path = nxtools::to_lower(normalized_path);
    }

    return traverse(nxtools::split(normalized_path.substr(1), '/'));
}

object *object::
traverse(vector_view<std::string> const &names) const
{
    if (!is_directory()) {
        errno = ENOTDIR;
        return nullptr;
    }

    if (names.empty())
        return reference(this);

    auto i = directory::get_entries().find(names[0]);
    if (i == directory::get_entries().end()) {
        errno = ENOENT;
        return nullptr;
    }

    auto o = _volume->open(i->second.oid);
    if (o == nullptr)
        return nullptr;

    if (!o->is_directory()) {
        return o;
    } else {
        auto result = o->traverse(names + 1);
        o->release();
        return result;
    }
}

object *object::
reference(object const *o) const
{
    auto &cache = _volume->get_cache();

    cache.lock();
    cache.add_unlocked(const_cast<object *>(o));
    cache.unlock();

    return const_cast<object *>(o);
}
