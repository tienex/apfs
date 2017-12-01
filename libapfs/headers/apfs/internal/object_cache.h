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

#ifndef __apfs_internal_object_cache_h
#define __apfs_internal_object_cache_h

#include "apfs/internal/base.h"

#include <mutex>

namespace apfs { class object; class volume; }

namespace apfs { namespace internal {

class object_cache {
private:
    struct entry {
        using oid_map = std::map<uint64_t, entry>;

        apfs::object *object;
        size_t        refs;
        time_t        expire;
    };

private:
    std::mutex      _lock;
    entry::oid_map  _oids;
    apfs::object   *_root;

protected:
    friend class apfs::volume;
    friend class apfs::object;

protected:
    void set_root(apfs::object *root);

protected:
    void lock();
    void unlock();

protected:
    void add_unlocked(apfs::object *o);
    apfs::object *reference_unlocked(uint64_t oid);

protected:
    void release(apfs::object *o);

protected:
    void clear();

private:
    void purge();
};

} }

#endif  // !__apfs_object_cache_h
