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

#include "volume.h"
#include "rsrcfork.h"
#include "xattr_file.h"

#include "nxtools/string.h"

#include <cerrno>

using apfs_fuse::volume;

volume::volume(apfs::volume *volume)
    : _volume(volume)
{
}

volume::~volume()
{
    delete _volume;
}

void volume::
stat(statfs_t *st, bool container) const
{
    _volume->stat(st, container);
}

#ifdef __APPLE__
void volume::
stat(struct statvfs *st, bool container) const
{
    _volume->stat(st, container);
}
#endif

char const *volume::
get_name() const
{
    return _volume->get_name();
}

nx_uuid_t const &volume::
get_uuid() const
{
    return _volume->get_uuid();
}

apfs_fuse::object *volume::
open(std::string const &path) const
{
    if (expose_xattr_directory || expose_resource_fork) {
        auto elements = nxtools::split(path, '/');

        //
        // If the last path element is ..xattr, open the
        // virtual xattr directory.
        //
        // Paths can be:
        //
        // ..xattr
        // 
        // Being the directory containing the directories
        // named after the files having xattrs)
        //
        // ..xattr/filename
        //
        // Being the directory containing the extended
        // attributes contained in filename.
        //
        // ..xattr/filename/xattrname
        //
        // The virtual file representing the extended
        // attribute
        //
        // 'filename' can be XATTR_ROOT_DIRECTORY if at
        // the root of the disk.
        //
        if (elements.back() == XATTR_DIRECTORY) {
            elements.pop_back();

            auto new_path = nxtools::join(elements, "/");

            auto o = _volume->open(new_path);
            if (o == nullptr)
                return nullptr;

            return new xattr_directory(o);
        } else {
            size_t nelements = elements.size();

            if (nelements > 1 && elements[nelements - 2] == XATTR_DIRECTORY) {
                //
                // Pointing to a directory inside the ..xattr directory.
                // Special handling is required for ..root directory.
                //
                elements.erase(elements.end() - 2);
                if (elements.back() == XATTR_ROOT_DIRECTORY) {
                    elements.pop_back();
                }

                auto new_path = nxtools::join(elements, "/");

                auto o = _volume->open(new_path);
                if (o == nullptr)
                    return nullptr;

                return new xattr_object_directory(o);
            } else if (nelements > 2 &&
                    elements[nelements - 3] == XATTR_DIRECTORY) {
                //
                // Pointing to the extended attribute inside the
                // ..xattr/filename directory.
                // Special handling is required for ..root directory.
                //
                auto xattr_name = elements.back();
                elements.erase(elements.end() - 3);
                elements.pop_back();

                if (elements.back() == XATTR_ROOT_DIRECTORY) {
                    elements.pop_back();
                }

                auto new_path = nxtools::join(elements, "/");

                auto o = _volume->open(new_path);
                if (o == nullptr)
                    return nullptr;

                return new xattr_file(o, xattr_name);
            }
        }

        //
        // If the last path element is ._name, open
        // name and then create a virtual file that
        // points to the resource fork.
        //
        if (unprefix_rsrc_name(elements.back())) {
            auto new_path = nxtools::join(elements, "/");

            //
            // Open the real object at the new path
            //
            auto o = _volume->open(new_path);
            if (o == nullptr)
                return nullptr;

            if (!o->has_xattr(APFS_XATTR_NAME_RESOURCEFORK)) {
                o->release();
                errno = ENOENT;
                return nullptr;
            }

            //
            // Create the rsrcfork
            //
            return new rsrcfork(o, expose_xattr_directory);
        }
    }

    auto o = _volume->open(path);
    if (o == nullptr)
        return nullptr;

    if (o->is_directory())
        return new directory(o);
    else
        return new file(o);
}

apfs_fuse::file *volume::
open_file(std::string const &path) const
{
    auto o = open(path);
    if (o == nullptr)
        return nullptr;

    if (!o->is_regular()) {
        errno = o->is_directory() ? EISDIR : EINVAL;
        delete o;
        return nullptr;
    }

    return static_cast<file *>(o);
}

apfs_fuse::directory *volume::
open_directory(std::string const &path) const
{
    auto o = open(path);
    if (o == nullptr)
        return nullptr;

    if (!o->is_directory()) {
        errno = ENOTDIR;
        delete o;
        return nullptr;
    }

    return static_cast<directory *>(o);
}
