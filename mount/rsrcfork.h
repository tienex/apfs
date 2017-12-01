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

#ifndef __apfs_fuse_rsrcfork_h
#define __apfs_fuse_rsrcfork_h

#include "file.h"

namespace apfs_fuse {

class rsrcfork : public file {
private:
    bool _xattrlink;

public:
    rsrcfork(apfs::object *o, bool xattrlink);

public:
    bool is_directory() const override;
    bool is_symbolic_link() const override;
    bool is_regular() const override;

public:
    int getattr(struct stat *st) const override;

public:
    ssize_t listxattr(char *namebuf, size_t size) const override;
    ssize_t getxattr(char const *name, void *buf, size_t size,
            uint32_t position) const override;

public:
    ssize_t read(void *buf, size_t size, off_t offset) const override;

public:
    int readlink(char *buf, size_t bufsize) const override;

public:
    uint64_t get_size() const override;
};

}

#endif  // !__apfs_fuse_rsrcfork_h
