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

#include "apfs/internal/object_cache.h"
#include "apfs/object.h"

#include <ctime>

using apfs::internal::object_cache;

void object_cache::
set_root(apfs::object *root)
{
    _root = root;
}

void object_cache::
add_unlocked(apfs::object *o)
{
    if (o == nullptr || o == _root)
        return;

    auto i = _oids.find(o->get_file_id());
    if (i != _oids.end()) {
        i->second.refs++;
        return;
    }

    entry e;
    e.refs   = 1;
    e.object = o;
    e.expire = 0;

    _oids[o->get_file_id()] = e;
}

void object_cache::
clear()
{
    std::lock_guard<std::mutex> _(_lock);

    for (auto &i : _oids) {
        delete i.second.object;
    }

    _oids.clear();
}

void object_cache::
purge()
{
    time_t now = time(nullptr);
    for (auto i = _oids.begin(); i != _oids.end();) {
        if (i->second.expire != 0 && now >= i->second.expire) {
            delete i->second.object;
            _oids.erase(i++);
        } else {
            ++i;
        }
    }
}

void object_cache::
lock()
{
    _lock.lock();
}

void object_cache::
unlock()
{
    _lock.unlock();
}

apfs::object *object_cache::
reference_unlocked(uint64_t oid)
{
    if (oid == APFS_DREC_ROOT_FILE_ID)
        return _root;
    if (oid < APFS_DREC_ROOT_FILE_ID)
        return nullptr;

    auto i = _oids.find(oid);
    if (i == _oids.end())
        return nullptr;

    i->second.refs++;
    i->second.expire = 0;

    // must be called locked
    purge();

    return i->second.object;
}

void object_cache::
release(apfs::object *o)
{
    if (o == nullptr || o == _root)
        return;

    std::lock_guard<std::mutex> _(_lock);

    auto i = _oids.find(o->get_file_id());
    if (i == _oids.end())
        return;

    if (i->second.refs == 0)
        return;

    if (--i->second.refs == 0) {
        // Will be deleted after some time, unless it's going to be used.
        i->second.expire = time(nullptr) + 10*60;
    }
}
