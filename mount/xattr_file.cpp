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

#include "xattr_file.h"

#include <sys/stat.h>

using apfs_fuse::xattr_file;

xattr_file::xattr_file(apfs::object *o, std::string const &xattr)
    : file  (o)
    , _xattr(xattr)
{
}

uint64_t xattr_file::
get_size() const
{
    return _object->get_xattr_size(_xattr);
}

bool xattr_file::
is_directory() const
{
    return false;
}

bool xattr_file::
is_symbolic_link() const
{
    return false;
}

bool xattr_file::
is_regular() const
{
    return true;
}

bool xattr_file::
is_virtual() const
{
    return true;
}

int xattr_file::
getattr(struct stat *st) const
{
    if (file::getattr(st) != 0)
        return -1;

    st->st_mode = S_IFREG | (st->st_mode & 0666);
    st->st_size = get_size();
    return 0;
}

ssize_t xattr_file::
read(void *buf, size_t size, off_t offset) const
{
    return _object->read_xattr(_xattr, buf, size, offset);
}
