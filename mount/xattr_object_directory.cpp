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

#include "xattr_object_directory.h"

#include <sys/stat.h>

using apfs_fuse::xattr_object_directory;

xattr_object_directory::xattr_object_directory(apfs::object *o)
    : directory(o, true) 
{
    o->get_xattrs(_xattrs);
}

bool xattr_object_directory::
is_directory() const
{
    return true;
}

bool xattr_object_directory::
is_symbolic_link() const
{
    return false;
}

bool xattr_object_directory::
is_regular() const
{
    return false;
}

bool xattr_object_directory::
is_virtual() const
{
    return true;
}

int xattr_object_directory::
getattr(struct stat *st) const
{
    directory::getattr(st);
    st->st_mode = S_IFDIR | (st->st_mode & 0777);
    if (st->st_mode & 0400) st->st_mode |= 0100;
    if (st->st_mode & 0040) st->st_mode |= 0010;
    if (st->st_mode & 0004) st->st_mode |= 0001;
    st->st_nlink = _object->get_xattr_count();
    st->st_size = 0;
    return 0;
}

bool xattr_object_directory::
rewind()
{
    return true;
}

bool xattr_object_directory::
next(off_t offset, std::string &name, uint64_t &file_id,
        off_t &next_offset)
{
    if (static_cast<size_t>(offset) >= _xattrs.size())
        return false;

    name = _xattrs[offset];
    next_offset = offset + 1;
    return true;
}
