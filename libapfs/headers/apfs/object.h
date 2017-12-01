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

#ifndef __apfs_object_h
#define __apfs_object_h

#include "apfs/session.h"
#include "apfs/internal/file.h"
#include "apfs/internal/directory.h"

namespace apfs { namespace internal { class object_cache; } }

namespace apfs {

class object : protected internal::file, protected internal::directory {
protected:
    volume *_volume;

public:
    struct info {
        nx_ino_t           ino;
        nx_mode_t          mode;
        nx_nlink_t         nlink;
        nx_uid_t           uid;
        nx_gid_t           gid;
        nx_dev_t           rdev;
        nx_off_t           size;
        struct nx_timespec atim;
        struct nx_timespec mtim;
        struct nx_timespec ctim;
        struct nx_timespec btim;
        nx_blksize_t       blksize;
        nx_blkcnt_t        blocks;
        uint32_t           flags;

        info()
        {
            ino = 0;
            mode = 0;
            nlink = 0;
            uid = 0;
            gid = 0;
            rdev = 0;
            size = 0;
            atim.tv_sec = 0;
            atim.tv_nsec = 0;
            mtim.tv_sec = 0;
            mtim.tv_nsec = 0;
            ctim.tv_sec = 0;
            ctim.tv_nsec = 0;
            btim.tv_sec = 0;
            btim.tv_nsec = 0;
            blksize = 0;
            blocks = 0;
            flags = 0;
        }

        uint16_t native_mode() const;
        dev_t native_dev() const;
        uint32_t native_flags() const;
    };

protected:
    friend class internal::object_cache;
    friend class volume;
    object();
    ~object();

protected:
    bool open(volume *volume, nx::enumerator *e);

public:
    void release();

public:
    inline uint64_t get_file_id() const
    { return file::get_oid(); }

public:
    inline bool is_regular() const
    { return ((file::get_mode() & APFS_INODE_MODE_IFMT) ==
            APFS_INODE_MODE_IFREG); }

    inline bool is_symbolic_link() const
    { return ((file::get_mode() & APFS_INODE_MODE_IFMT) ==
            APFS_INODE_MODE_IFLNK); }

    inline bool is_directory() const
    { return ((file::get_mode() & APFS_INODE_MODE_IFMT) ==
            APFS_INODE_MODE_IFDIR); }

public:
    inline std::string const &get_name() const
    { return _name; }
    void get_info(info &info) const;
    void stat(struct stat *st) const;

public:
    enum {
        HAS_XATTR_THIS = 1,
        HAS_XATTR_DESCENDENT = 2,
        HAS_XATTR_ANY = HAS_XATTR_THIS | HAS_XATTR_DESCENDENT
    };

    bool has_xattrs(unsigned flags = HAS_XATTR_THIS) const;
    size_t get_xattr_count() const;
    void get_xattrs(string_vector &xattrs) const;
    bool has_xattr(std::string const &name) const;
    uint64_t get_xattr_size(std::string const &name) const;
    ssize_t read_xattr(std::string const &name, void *buf, size_t size,
            nx_off_t offset) const;

public:
    bool read_symbolic_link(std::string &value) const;

public:
    ssize_t read(void *buf, size_t size, nx_off_t offset) const;

public:
    object *traverse(std::string const &path) const;

private:
    object *traverse(vector_view<std::string> const &names) const;

public:
    using directory_entry_map = directory::entry::name_map;
    directory_entry_map const &get_entries() const
    { return directory::get_entries(); }

public:
    inline uint64_t get_size() const
    { return file::get_size(); }
    inline uint64_t get_alloced_size() const
    { return file::get_alloced_size(); }

public:
    inline bool is_root() const
    { return (get_file_id() == APFS_DREC_ROOT_FILE_ID); }

private:
    object *reference(object const *o) const;

public:
    inline volume *get_volume() const
    { return _volume; }
};

}

#endif  // !__apfs_object_h
