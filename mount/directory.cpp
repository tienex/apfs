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

#include <cassert>

using apfs_fuse::directory;

directory::directory(apfs::object *o, bool noxattr)
    : object  (o)
    , _entries(_object->get_entries())
    , _noxattr(noxattr)
{
}

bool directory::
rewind()
{
    _iterator = _entries.begin();
    _doffset = 0;

    return true;
}

bool directory::
next(off_t offset, std::string &name, uint64_t &file_id, off_t &next_offset)
{
    int rfoffset = 0;

    if (offset != _doffset)
        return false;

    bool at_eod = (_iterator == _entries.end());

    if (!_noxattr && expose_xattr_directory) {
        rfoffset = 1;
        if (_doffset == 0) {
            name = XATTR_DIRECTORY;
            next_offset = ++_doffset;
            return true;
        }
    }

    if (!_noxattr && expose_resource_fork) {
        if ((_doffset & 1) == rfoffset) {
            //
            // Every other file, we return the resource fork
            // synthesized. This is slow, of course.
            //

            bool has_rsrc_fork = false;

            if (at_eod) {
                //
                // Simulate entry for the root directory, but only
                // if it's really the root.
                //
                if (!_object->is_root())
                    return false;

                if (_doffset > _entries.size() * 2 + rfoffset)
                    return false;

                has_rsrc_fork = _object->has_xattr(APFS_XATTR_NAME_RESOURCEFORK);
                name          = "";
                file_id       = _object->get_file_id();
            } else {
                auto *o = _object->get_volume()->open(_iterator->second.oid);
                if (o != nullptr) {
                    has_rsrc_fork = o->has_xattr(APFS_XATTR_NAME_RESOURCEFORK);
                    if (has_rsrc_fork) {
                        name    = _iterator->second.name;
                        file_id = _iterator->second.oid;
                    }
                    o->release();
                }
            }

            if (has_rsrc_fork) {
                name        = prefix_rsrc_name(name);
                file_id     = INT64_MAX ^ file_id;
                next_offset = ++_doffset;
                return true;
            }

            //
            // Assume we incremented _doffset.
            //
            ++_doffset;
        }

        assert((_doffset & 1) == !rfoffset && "_doffset is not paired up");
    }

    if (at_eod) {
        //
        // At the end.
        //
        return false;
    }

    name        = _iterator->second.name;
    file_id     = _iterator->second.oid;
    next_offset = ++_doffset;

    ++_iterator;

    return true;
}
