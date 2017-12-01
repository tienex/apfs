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

#ifndef __apfs_internal_file_h
#define __apfs_internal_file_h

#include "apfs/internal/xattr.h"

struct stat;

namespace apfs { namespace internal {

class file : public object {
protected:
    apfs_inode_value_t _inode;
    xattr::vector      _xattrs;

public:
    file();

protected:
    void set_inode(void const *v, size_t vsize);
    void add_xattr(void const *k, void const *v);

public:
    bool is_symbolic_link() const;
    std::string get_symbolic_link() const;

protected:
    inline apfs_inode_value_t const &get_inode() const
    { return _inode; }
    inline uint16_t get_mode() const
    { return _inode.mode; }

public:
    inline xattr::vector &get_xattrs()
    { return _xattrs; }
    inline xattr::vector const &get_xattrs() const
    { return _xattrs; }

private:
    static apfs_inode_value_t swap_inode(apfs_inode_value_t const &in);
};

} }

#endif  // !__apfs_internal_file_h
