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

#include "directory.h"

#include <cerrno>
#include <cstring>

#if defined(__FreeBSD__)
//
// FreeBSD requires xattrs to be prefixed with the namespace.
//
#define XATTR_NAMESPACE     "user."
#define XATTR_NAMESPACE_LEN 5
#else
#define XATTR_NAMESPACE     ""
#define XATTR_NAMESPACE_LEN 0
#endif

using apfs_fuse::object;

object::object(apfs::object *o)
    : _object(o)
{
}

object::~object()
{
    if (_object != nullptr) {
        _object->release();
    }
}

int object::
getattr(struct stat *st) const
{
    _object->stat(st);
    return 0;
}

ssize_t object::
listxattr(char *namebuf, size_t size) const
{
    apfs::string_vector names;

    _object->get_xattrs(names);
    if (names.empty())
        return 0;

    //
    // On macOS there's a bug, if we're at root, any size we return
    // will require com.apple.FinderInfo to be present, if we have it,
    // we won't return it.
    //
    size_t required_size = 0;
    for (auto &n : names) {
#if defined(__APPLE__)
        if (novolicon_unsupported && _object->is_root() &&
                n == APFS_XATTR_NAME_FINDERINFO)
            continue;
#endif
        required_size += n.length() + XATTR_NAMESPACE_LEN + 1;
    }

    if (size == 0) {
        return required_size;
    } else if (size < required_size) {
        errno = E2BIG;
        return -1;
    }

    char *p = namebuf, *end = namebuf + size;
    for (auto &n : names) {
#if defined(__APPLE__)
        if (novolicon_unsupported && _object->is_root() &&
                n == APFS_XATTR_NAME_FINDERINFO)
            continue;
#endif
        std::string xattr_name = XATTR_NAMESPACE + n;
        memcpy(p, xattr_name.c_str(), xattr_name.length() + 1);
        p += xattr_name.length() + 1;
    }

    return required_size;
}

ssize_t object::
getxattr(char const *name, void *buf, size_t size, uint32_t position) const
{
#if (XATTR_NAMESPACE_LEN > 0)
    if (strncmp(name, XATTR_NAMESPACE, XATTR_NAMESPACE_LEN) != 0) {
        errno = ENOATTR;
        return -1;
    }
    name += XATTR_NAMESPACE_LEN;
#endif

    if (size == 0) {
        return _object->get_xattr_size(name);
    }

    if (buf == nullptr) {
        errno = EFAULT;
        return -1;
    }

    return _object->read_xattr(name, buf, size, position);
}

int object::
readlink(char *buf, size_t bufsize) const
{
    if (!_object->is_symbolic_link()) {
        errno = EINVAL;
        return -1;
    }

    std::string value;
    if (!_object->read_symbolic_link(value)) {
        errno = EIO;
        return -1;
    }

    if (bufsize < value.length()) {
        errno = E2BIG;
        return -1;
    }

    memcpy(buf, value.c_str(), value.length() + 1);
    return 0;
}

bool object::
is_directory() const
{
    return _object->is_directory();
}

bool object::
is_symbolic_link() const
{
    return _object->is_symbolic_link();
}

bool object::
is_regular() const
{
    return _object->is_regular();
}

bool object::
is_virtual() const
{
    return false;
}
