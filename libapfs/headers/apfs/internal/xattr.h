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

#ifndef __apfs_internal_xattr_h
#define __apfs_internal_xattr_h

#include "apfs/internal/object.h"

namespace apfs { class object; }

namespace apfs { namespace internal {

class xattr : public object {
public:
    using vector = std::vector<xattr>;

protected:
    byte_vector _content;

public:
    xattr();

protected:
    void set_inline_content(byte_vector &&content);

protected:
    friend class file;
    friend class apfs::object;
    inline bool is_content_inlined() const
    { return (_oid == 0); }
    byte_vector const &get_inline_content() const
    { return _content; }

public:
    virtual uint64_t get_size() const override;
    virtual uint64_t get_alloced_size() const override;

public:
    virtual ssize_t read(nx::device *device, void *buffer, size_t size,
            nx_off_t offset) const override;
};

} }

#endif  // !__apfs_internal_xattr_h
