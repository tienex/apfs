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

#ifndef __apfs_internal_object_h
#define __apfs_internal_object_h

#include "apfs/internal/extent.h"

namespace apfs { namespace internal {

class object {
protected:
    uint64_t       _oid;
    std::string    _name;
    extent::vector _extents;
    apfs_dstream_t _dstream;
    uint32_t       _device;

protected:
    object();

protected:
    friend class file;

    void set_oid(uint64_t oid);
    void set_dstream(apfs_dstream_t const &dstream);
    void set_device_spec(uint32_t device);
    void set_name(std::string &&name);

    void add_extent(void const *k, void const *v);

public:
    inline uint64_t get_oid() const
    { return _oid; }
    virtual inline uint64_t get_size() const
    { return _dstream.dstream_size; }
    virtual inline uint64_t get_alloced_size() const
    { return _dstream.dstream_alloced_size; }
    inline std::string const &get_name() const
    { return _name; }
    inline uint32_t get_device_spec() const
    { return _device; }

private:
    bool offset_to_extent(nx_off_t offset, uint64_t &lba, size_t &count,
            size_t &loffset) const;

public:
    virtual ssize_t read(nx::device *device, void *buffer, size_t size,
            nx_off_t offset) const;

protected:
    static apfs_dstream_t swap_dstream(apfs_dstream_t const &in);
};

} }

#endif  // !__apfs_internal_object_h
