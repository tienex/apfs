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

#include "apfs/internal/object.h"

#include <cstring>

using apfs::internal::object;

object::object()
    : _oid(0)
{
    memset(&_dstream, 0, sizeof(_dstream));
}

void object::
set_oid(uint64_t oid)
{
    _oid = oid;
}

void object::
set_dstream(apfs_dstream_t const &dstream)
{
    _dstream = dstream;
}

void object::
set_name(std::string &&name)
{
    _name = std::move(name);
}

void object::
set_device_spec(uint32_t device)
{
    _device = device;
}

void object::
add_extent(void const *k, void const *v)
{
    auto fek = reinterpret_cast<apfs_file_extent_key_t const *>(k);
    auto fev = reinterpret_cast<apfs_file_extent_value_t const *>(v);

    _extents.push_back(
            extent(nx::swap(fek->offset) / NX_OBJECT_SIZE,
                   nx::swap(fev->phys_block_num),
                   APFS_FILE_EXTENT_VALUE_LENGTH(fev) / NX_OBJECT_SIZE));
}

bool object::
offset_to_extent(nx_off_t offset, uint64_t &lba, size_t &count,
        size_t &loffset) const
{
    if (offset >= get_size())
        return false;

    uint64_t bno = offset / NX_OBJECT_SIZE;

    for (auto &e : _extents) {
        if (bno >= e.offset && bno < e.offset + e.count) {
            lba = e.lba + (bno - e.offset);
            count = e.count;
            loffset = offset % NX_OBJECT_SIZE;
            return true;
        }
    }

    return false;
}

ssize_t object::
read(nx::device *device, void *buf, size_t size, nx_off_t offset) const
{
    uint64_t lba;
    size_t   count;
    size_t   loffset;
    uint8_t *block;
    uint8_t *base  = reinterpret_cast<uint8_t *>(buf);
    uint8_t *bytes = base;

    if (size == 0 || offset < 0 || offset >= get_size())
        return 0;

    if (size + offset >= get_size()) {
        size = get_size() - offset;
        if (size == 0)
            return 0;
    }

    block = device->new_block<uint8_t>();
    if (block == nullptr) {
        errno = ENOMEM;
        return -1;
    }

    while (size > 0) {
        if (!offset_to_extent(offset, lba, count, loffset))
            break;

        while (size > 0 && count-- != 0) {
            size_t len = std::min(size,
                    static_cast<size_t>(NX_OBJECT_SIZE) - loffset);

            if (!device->read(lba++, block, 1, nullptr))
                break;

            memcpy(bytes, block + loffset, len);
            bytes += len, offset += len, size -= len, loffset = 0;
        }
    }

    nx::device::free_block(block);

    return bytes - base;
}

apfs_dstream_t object::
swap_dstream(apfs_dstream_t const &in)
{
    return {
        .dstream_size              = nx::swap(in.dstream_size),
        .dstream_alloced_size      = nx::swap(in.dstream_alloced_size),
        .dstream_default_crypto_id = nx::swap(in.dstream_default_crypto_id),
        .dstream_uuid              = in.dstream_uuid
    };
}
