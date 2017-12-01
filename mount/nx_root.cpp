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

#include "nx_root.h"
#include "nx_volume.h"

#include <sys/stat.h>

using apfs_fuse::nx_root;

nx_root::nx_root(nx_volume const *volume)
    : directory(nullptr)
    , _volume  (volume)
{
    rewind();
}

bool nx_root::
is_directory() const
{
    return true;
}

bool nx_root::
is_symbolic_link() const
{
    return false;
}

bool nx_root::
is_regular() const
{
    return false;
}

bool nx_root::
is_virtual() const
{
    return true;
}

int nx_root::
getattr(struct stat *st) const
{
    st->st_nlink = _volume->get_volumes().size();
    st->st_mode  = S_IFDIR | 0755;
    st->st_size  = st->st_nlink;
    st->st_mtime = time(nullptr);
    st->st_ctime = time(nullptr);
    st->st_atime = time(nullptr);
    return 0;
}

bool nx_root::
rewind()
{
    _iterator = _volume->get_volumes().begin();
    return true;
}

bool nx_root::
next(off_t offset, std::string &name, uint64_t &file_id, off_t &next_offset)
{
    if (_iterator == _volume->get_volumes().end())
        return false;

    name = _iterator->first;
    next_offset = offset + 1;
    ++_iterator;

    return true;
}
