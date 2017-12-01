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

#ifndef __apfs_fuse_volume_h
#define __apfs_fuse_volume_h

#include "file.h"
#include "directory.h"

namespace apfs_fuse {

#ifdef __APPLE__
using statfs_t = struct statfs;
#else
using statfs_t = struct statvfs;
#endif

class volume {
private:
    apfs::volume *_volume;

public:
    volume(apfs::volume *volume);
    ~volume();

public:
    virtual void stat(statfs_t *st, bool container = false) const;
#ifdef __APPLE__
    virtual void stat(struct statvfs *st, bool container = false) const;
#endif

public:
    virtual char const *get_name() const;
    virtual nx_uuid_t const &get_uuid() const;

public:
    virtual object *open(std::string const &path) const;
    virtual file *open_file(std::string const &path) const;
    virtual directory *open_directory(std::string const &path) const;
};

}

#endif  // !__apfs_fuse_volume_h
