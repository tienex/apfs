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

#include "apfs/volume.h"
#include "apfs/object.h"

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif

#include <cerrno>

using apfs::volume;

volume::volume()
    : _session(nullptr)
    , _volume (nullptr)
    , _root   (nullptr)
{
}

volume::~volume()
{
    close();
}

bool volume::
open(session *session, size_t volid)
{
    if (session == nullptr) {
        errno = EFAULT;
        return false;
    }

    if (_session != nullptr) {
        errno = EINVAL;
        return false;
    }

    auto container = session->get_container();
    _volume = container->open_volume(volid);
    if (_volume == nullptr) {
        errno = EINVAL;
        return false;
    }

    auto root = _volume->open_root();
    if (root == nullptr) {
        close();
        errno = EINVAL;
        return false;
    }

    _root = new object;
    bool success = _root->open(this, root);
    delete root;

    if (!success) {
        close();
        errno = EINVAL;
    }

    _cache.set_root(_root);
    _session = session;
    return true;
}

void volume::
close()
{
    delete _root;
    delete _volume;
    _root = nullptr;
    _volume = nullptr;
    _session = nullptr;
}

void volume::
stat(struct statvfs *st, bool container)
{
#ifdef HAVE_STATVFS
    auto c = _session->get_container();

    st->f_bsize   = NX_OBJECT_SIZE;
    if (container) {
        nx::container::info info;
        if (c->get_info(info)) {
            st->f_frsize = info.blksize;
            st->f_blocks = info.blocks;
            st->f_bavail = info.bfree;
            st->f_bfree  = info.bfree;
            st->f_files  = info.volumes;
            st->f_ffree  = info.vfree;
            st->f_favail = info.vfree;
        }
    } else {
        st->f_frsize  = c->get_block_size();
        st->f_blocks  = c->get_block_count();
        st->f_bavail  = _volume->get_free_block_count();
        st->f_bfree   = _volume->get_available_block_count();
        st->f_files   = static_cast<decltype(st->f_files)>(INT64_MAX);
        st->f_ffree   = static_cast<decltype(st->f_ffree)>(INT64_MAX -
                _volume->get_inode_used_count());
        st->f_favail  = st->f_ffree;
    }
#if defined(ST_RDONLY)
#if defined(HAVE_STATFS_F_FLAG)
    st->f_flag    = ST_RDONLY;
#elif defined(HAVE_STATFS_F_FLAGS)
    st->f_flags   = ST_RDONLY;
#endif
#endif
#else
    (void)st;
#endif
}

void volume::
stat(struct statfs *st, bool container)
{
#ifdef HAVE_STATFS
    auto c = _session->get_container();
    nx::container::info info;

    if (container) {
        c->get_info(info);
    }

#ifdef HAVE_STATFS_F_IOSIZE
    st->f_iosize  = NX_OBJECT_SIZE;
    // f_bsize means really block size
    st->f_bsize   = container ? info.blksize : c->get_block_size();
#else
    // f_bsize means optimal i/o size
    st->f_bsize   = NX_OBJECT_SIZE;
#endif
#ifdef HAVE_STATFS_F_FRSIZE
    // st_frsize is the size of a block
    st->f_frsize  = container ? info.blksize : c->get_block_size();
#endif
    st->f_blocks  = container ? info.blocks : c->get_block_count();
#ifdef HAVE_STATFS_F_BAVAIL
    st->f_bavail  = container ? info.bfree : _volume->get_free_block_count();
#endif
    st->f_bfree   = container ? info.bfree : _volume->get_available_block_count();
#if !defined(HAVE_STATFS_F_IOSIZE) && !defined(HAVE_STATFS_F_FRSIZE)
    // Unit is 512-bytes
    st->f_blocks  = (st->f_blocks * c->get_block_size()) / 512;
#ifdef HAVE_STATFS_F_BAVAIL
    st->f_bavail  = (st->f_bavail * c->get_block_size()) / 512;
#endif
    st->f_bfree   = (st->f_bfree  * c->get_block_size()) / 512;
#endif
    if (container) {
        st->f_files   = info.volumes;
        st->f_ffree   = info.vfree;
    } else {
        st->f_files   = static_cast<decltype(st->f_files)>(INT64_MAX);
        st->f_ffree   = static_cast<decltype(st->f_ffree)>(INT64_MAX -
                _volume->get_inode_used_count());
    }
#if defined(ST_RDONLY)
#if defined(HAVE_STATFS_F_FLAG)
    st->f_flag    = ST_RDONLY;
#elif defined(HAVE_STATFS_F_FLAGS)
    st->f_flags   = ST_RDONLY;
#endif
#endif
#else
    (void)st;
#endif
}

apfs::object *volume::
open_root()
{
    return _root;
}

apfs::object *volume::
open(uint64_t oid)
{
    if (oid < APFS_DREC_ROOT_FILE_ID) {
        errno = ENOENT;
        return nullptr;
    }

    if (oid == APFS_DREC_ROOT_FILE_ID)
        return open_root();

    _cache.lock();
    auto o = _cache.reference_unlocked(oid);
    if (o == nullptr) {
        auto e = _volume->open_oid(oid);
        if (e != nullptr) {
            o = new object;
            if (!o->open(this, e)) {
                delete o;
                o = nullptr;
            } else {
                _cache.add_unlocked(o);
            }
            delete e;
        }
    }
    _cache.unlock();

    return o;
}

apfs::object *volume::
open(std::string const &path)
{
    return _root->traverse(path);
}
