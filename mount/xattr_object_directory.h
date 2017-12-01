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

#ifndef __apfs_fuse_xattr_object_directory_h
#define __apfs_fuse_xattr_object_directory_h

#include "directory.h"

namespace apfs_fuse {

class xattr_object_directory : public directory {
private:
    apfs::string_vector _xattrs;

protected:
    friend class volume;
    xattr_object_directory(apfs::object *o);

public:
    bool is_directory() const override;
    bool is_symbolic_link() const override;
    bool is_regular() const override;
    bool is_virtual() const override;

public:
    int getattr(struct stat *st) const override;

public:
    bool rewind() override;
    bool next(off_t offset, std::string &name, uint64_t &file_id,
            off_t &next_offset) override;
};

}

#endif  // !__apfs_fuse_xattr_object_directory_h
