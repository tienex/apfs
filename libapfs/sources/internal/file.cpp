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

#include "apfs/internal/file.h"

#include <cstring>

using apfs::internal::file;

file::file()
    : object()
{
    memset(&_inode, 0, sizeof(_inode));
}

void file::
set_inode(void const *v, size_t vsize)
{
    auto iv = reinterpret_cast<apfs_inode_value_t const *>(v);

    //
    // Copy and swap fields.
    //
    _inode = swap_inode(*iv);

    //
    // Read xfields.
    //
    if (vsize > sizeof(*iv)) {
        auto xf      = reinterpret_cast<apfs_xfield_t const *>(iv + 1);
        auto xf_data = reinterpret_cast<uint8_t const *>(APFS_XFIELD_DATA(xf));

        for (size_t n = 0; n < nx::swap(xf->xf_num_exts); n++) {
            size_t xf_data_len = APFS_XFIELD_ALIGN_LENGTH(
                    nx::swap(xf->xf_entries[n].xf_len));

            switch (xf->xf_entries[n].xf_type) {
                case APFS_INO_EXT_TYPE_NAME:
                    {
                        auto name = std::string(
                                reinterpret_cast<char const *>(xf_data),
                                xf_data_len);
                        name.resize(strlen(name.c_str()));
                        object::set_name(std::move(name));
                    }
                    break;

                case APFS_INO_EXT_TYPE_DSTREAM:
                    {
                        auto ds =
                            reinterpret_cast<apfs_dstream_t const *>(xf_data);

                        object::set_dstream(object::swap_dstream(*ds));
                    }
                    break;

                case APFS_INO_EXT_TYPE_DEVICE:
                    {
                        auto dev = reinterpret_cast<uint32_t const *>(xf_data);

                        object::set_device_spec(nx::swap(*dev));
                    }
                    break;
            }
            xf_data += xf_data_len;
        }
    }

    _oid = _inode.private_id;
}

void file::
add_xattr(void const *k, void const *v)
{
    auto xk  = reinterpret_cast<apfs_xattr_key_t const *>(k);
    auto xv  = reinterpret_cast<apfs_xattr_value_t const *>(v);
    auto xiv = reinterpret_cast<apfs_xattr_indirect_value_t const *>(v);

    auto name = std::string(xk->name, nx::swap(xk->name_len));
    name.resize(strlen(name.c_str()));

    xattr x;
    x.set_name(std::move(name));

    if (nx::swap(xv->flags) & APFS_XATTR_VALUE_FLAG_INDIRECT) {
        //
        // Copy oid and dstream
        //
        x.set_oid(nx::swap(xiv->xattr_obj_id));
        x.set_dstream(object::swap_dstream(xiv->dstream));
    } else {
        //
        // Content is inlined.
        //
        byte_vector content(nx::swap(xv->xdata_len));
        memcpy(&content[0], xv + 1, xv->xdata_len);
        x.set_inline_content(std::move(content));
    }

    _xattrs.push_back(std::move(x));
}

bool file::
is_symbolic_link() const
{
    for (auto &x : _xattrs) {
        if (x.get_name() == APFS_XATTR_NAME_SYMLINK)
            return true;
    }
    return false;
}

std::string file::
get_symbolic_link() const
{
    for (auto &x : _xattrs) {
        if (x.get_name() == APFS_XATTR_NAME_SYMLINK)
            return reinterpret_cast<char const *>(&x.get_inline_content()[0]);
    }
    return std::string();
}

apfs_inode_value_t file::
swap_inode(apfs_inode_value_t const &in)
{
    return {
        .parent_id                = nx::swap(in.parent_id),
        .private_id               = nx::swap(in.private_id),
        .creation_timestamp       = nx::swap(in.creation_timestamp),
        .modification_timestamp   = nx::swap(in.modification_timestamp),
        .changed_timestamp        = nx::swap(in.changed_timestamp),
        .access_timestamp         = nx::swap(in.access_timestamp),
        .internal_flags           = nx::swap(in.internal_flags),
        .nchildren                = nx::swap(in.nchildren),
        .default_protection_class = nx::swap(in.default_protection_class),
        .unknown_x40              = nx::swap(in.unknown_x40),
        .bsd_flags                = nx::swap(in.bsd_flags),
        .user_id                  = nx::swap(in.user_id),
        .group_id                 = nx::swap(in.group_id),
        .mode                     = nx::swap(in.mode),
        .pad1                     = nx::swap(in.pad1),
        .pad2                     = nx::swap(in.pad2)
    };
}
