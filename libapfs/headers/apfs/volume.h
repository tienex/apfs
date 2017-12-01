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

#ifndef __apfs_volume_h
#define __apfs_volume_h

#include "apfs/session.h"
#include "apfs/internal/file.h"
#include "apfs/internal/directory.h"
#include "apfs/internal/object_cache.h"

struct statfs;
struct statvfs;

namespace apfs {

class volume {
private:
    session                *_session;
    nx::volume             *_volume;
    object                 *_root;
    internal::object_cache  _cache;

public:
    volume();
    ~volume();

protected:
    friend class session;
    bool open(session *session, size_t volid);
    void close();

public:
    void stat(struct statvfs *st, bool container = false);
    void stat(struct statfs *st, bool container = false);

public:
    inline session *get_session() const
    { return _session; }

protected:
    friend class object;
    inline nx::volume *get_nx_volume() const
    { return _volume; }
    internal::object_cache &get_cache()
    { return _cache; }

public:
    inline bool is_case_sensitive() const
    { return _volume->is_case_sensitive(); }

public:
    inline size_t get_block_size() const
    { return _volume->get_block_size(); }

public:
    inline nx_uuid_t const &get_uuid() const
    { return _volume->get_uuid(); }
    inline char const *get_name() const
    { return _volume->get_name(); }

public:
    object *open_root();
    object *open(uint64_t oid);
    object *open(std::string const &path);
};

}

#endif  // !__apfs_volume_h
