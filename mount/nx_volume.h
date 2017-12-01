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

#ifndef __apfs_fuse_nx_volume_h
#define __apfs_fuse_nx_volume_h

#include "volume.h"
#include "nx_root.h"

namespace apfs_fuse {

class nx_volume : public volume {
private:
    apfs::session *_session;
    nx_volume_map  _volumes;
    std::string    _name;

public:
    nx_volume(apfs::session *session);
    ~nx_volume();

protected:
    friend class nx_root;
    inline nx_volume_map const &get_volumes() const
    { return _volumes; }

public:
    void stat(statfs_t *st, bool = false) const override;
#ifdef __APPLE__
    void stat(struct statvfs *st, bool = false) const override;
#endif

public:
    char const *get_name() const override;
    nx_uuid_t const &get_uuid() const override;

public:
    object *open(std::string const &path) const override;
    file *open_file(std::string const &path) const override;
    directory *open_directory(std::string const &path) const override;

private:
    volume *get_volume_path(std::string const &path,
            std::string &subpath) const;
};

}

#endif  // !__apfs_fuse_nx_volume_h
