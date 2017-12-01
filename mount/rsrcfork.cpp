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

#include "rsrcfork.h"

#include "nxcompat/nxcompat.h"

#include <sys/stat.h>

using apfs_fuse::rsrcfork;

rsrcfork::rsrcfork(apfs::object *o, bool xattrlink)
    : file      (o)
    , _xattrlink(xattrlink)
{
}

bool rsrcfork::
is_directory() const
{
    return false;
}

bool rsrcfork::
is_symbolic_link() const
{
    return _xattrlink;
}

bool rsrcfork::
is_regular() const
{
    return !_xattrlink;
}

int rsrcfork::
getattr(struct stat *st) const
{
    //
    // Inherit all attributes from underlying file/directory.
    //
    file::getattr(st);

    if (_xattrlink) {
        //
        // Ensure we are seen as a symbolic link.
        //
        st->st_mode = S_IFLNK | 0777;

        //
        // Size is the length of the link.
        //
        st->st_size = get_size();
    } else {
        //
        // Ensure we are seen as a regular file.
        //
        st->st_mode = S_IFREG | (st->st_mode & 0777);

        //
        // Size is the resource fork size.
        //
        st->st_size = get_size();
    }

    st->st_nlink = 1;

    return 0;
}

ssize_t rsrcfork::
listxattr(char *namebuf, size_t size) const
{
    //
    // No ext attrs for ext attrs!
    //
    return 0;
}

ssize_t rsrcfork::
getxattr(char const *name, void *buf, size_t size, uint32_t position) const
{
    //
    // No ext attrs for ext attrs!
    //
    errno = ENOATTR;
    return -1;
}

ssize_t rsrcfork::
read(void *buf, size_t size, off_t offset) const
{
    if (_xattrlink) {
        errno = EINVAL;
        return -1;
    }

    //
    // Read through super.
    //
    return file::getxattr(APFS_XATTR_NAME_RESOURCEFORK, buf, size, offset);
}

int rsrcfork::
readlink(char *buf, size_t bufsize) const
{
    if (!_xattrlink) {
        errno = EINVAL;
        return -1;
    }

    char const *name = _object->get_name().c_str();
    if (_object->is_root()) {
        name = XATTR_ROOT_DIRECTORY;
    }

    snprintf(buf, bufsize, "%s/%s/%s", XATTR_DIRECTORY, name,
            APFS_XATTR_NAME_RESOURCEFORK);

    return 0;
}

uint64_t rsrcfork::
get_size() const
{
    if (_xattrlink) {
        size_t name_len = _object->get_name().length();
        if (_object->is_root()) {
            name_len = XATTR_ROOT_DIRECTORY_LEN;
        }
        return (XATTR_DIRECTORY_LEN + 1 + name_len + 1 +
                strlen(APFS_XATTR_NAME_RESOURCEFORK));
    }

    return _object->get_xattr_size(APFS_XATTR_NAME_RESOURCEFORK);
}
