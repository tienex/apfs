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

#include "apfs/session.h"
#include "apfs/volume.h"

#include <cerrno>

using apfs::session;
using apfs::volume;

session::session(nx::context *context)
    : _context  (context)
    , _container(nullptr)
{
    if (_context == nullptr) {
        _context = new nx::context;
        _free_context = true;
    } else {
        _free_context = false;
    }
}

session::~session()
{
    stop();
    if (_free_context) {
        delete _context;
    }
}

bool session::
start_at(uint64_t xid)
{
    if (_container != nullptr) {
        errno = EINVAL;
        return false;
    }

    auto container = new nx::container(_context);
    if (!container->open_at(xid)) {
        delete container;
        errno = ENOENT;
        return false;
    }

    _container = container;
    return true;
}

bool session::
start(bool last_xid)
{
    if (_container != nullptr) {
        errno = EINVAL;
        return false;
    }

    auto container = new nx::container(_context);
    if (!container->open(last_xid)) {
        delete container;
        errno = ENOENT;
        return false;
    }

    _container = container;
    return true;
}

void session::
stop()
{
    if (_container == nullptr)
        return;

    for (auto &i : _volumes) {
        delete i.second;
    }

    _volumes.clear();
    //_cache.clear();

    delete _container;
    _container = nullptr;
}

volume *session::
open(size_t volid)
{
    auto i = _volumes.find(volid);
    if (i != _volumes.end())
        return i->second;

    auto v = new volume;
    if (!v->open(this, volid)) {
        delete v;
        return nullptr;
    }

    _volumes[volid] = v;
    return v;
}
