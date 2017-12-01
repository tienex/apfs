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

#include "xattr_directory.h"

using apfs_fuse::xattr_directory;

xattr_directory::xattr_directory(apfs::object *o)
    : directory(o, true)
    , _eod     (false)
{
}

bool xattr_directory::
is_virtual() const
{
    return true;
}

bool xattr_directory::
rewind()
{
    if (directory::rewind()) {
        _eod = false;
        return true;
    }
    return false;
}

bool xattr_directory::
next(off_t offset, std::string &name, uint64_t &file_id, off_t &next_offset)
{
    if (_eod)
        return false;

    if (!directory::next(offset, name, file_id, next_offset)) {
        if (_object->is_root()) {
            name = XATTR_ROOT_DIRECTORY;
            next_offset = offset + 1;
        }
        _eod = true;
    }

    return true;
}
