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

#include "nx/device.h"

#include "nxcompat/nxcompat.h"

#include <cstdlib>

using nx::device;

device::device()
    : _fd         (-1)
    , _block_size (0)
    , _block_count(0)
{
}

device::~device()
{
    close();
}

bool device::
open(char const *device, size_t block_size)
{
    int fd;
    struct stat stbuf;

    if (_fd >= 0) {
        errno = EINVAL;
        return false;
    }

    if (block_size == 0) {
        block_size = NX_OBJECT_SIZE;
    } else if (block_size & (block_size - 1)) {
        errno = EINVAL;
        return false;
    }

    fd = ::open(device, O_RDONLY | O_BINARY);
    if (fd < 0)
        return false;

    if (::fstat(fd, &stbuf) < 0) {
        ::close(fd);
        return false;
    }

    if (stbuf.st_size == 0) {
        uint64_t block_count;
        size_t   block_size;
        if (::fblkinfo(fd, &block_count, &block_size) < 0) {
            ::close(fd);
            return false;
        }
        stbuf.st_size = block_count * static_cast<uint64_t>(block_size);
    }

    _block_count = stbuf.st_size / block_size;
    if (_block_count == 0) {
        ::close(fd);
        errno = ENXIO;
        return false;
    }

    _block_size = block_size;
    _fd = fd;

    return true;
}

void device::
close()
{
    if (_fd < 0)
        return;

    ::close(_fd);
    _fd = -1;
    _block_size = 0;
    _block_count = 0;
}

bool device::
read(uint64_t lba, void *blocks, size_t count, size_t *nread) const
{
    ssize_t read_count;

    if (_fd < 0) {
        errno = EBADF;
        return false;
    }

    if (lba >= _block_count) {
        errno = E2BIG;
        return false;
    }

    if (lba + count >= _block_count) {
        count = _block_count - lba;
    }

    if (count == 0) {
        if (nread != nullptr) {
            *nread = 0;
        }
        return true;
    }

    if (blocks == nullptr) {
        errno = EFAULT;
        return false;
    }

    read_count = ::pread(_fd, blocks, count * (uint64_t)_block_size,
            lba * (uint64_t)_block_size);
    if (read_count < 0)
        return false;

    if (nread == nullptr) {
        if (read_count != (ssize_t)(count * (uint64_t)_block_size)) {
            errno = EIO;
            return false;
        }
    } else {
        *nread = read_count / _block_size;
    }

    return true;
}
