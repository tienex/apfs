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

#ifndef __nx_device_h
#define __nx_device_h

#include "nx/format/nx.h"

#include <cerrno>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <memory>

namespace nx {

class device {
private:
    int      _fd;
    size_t   _block_size;
    uint64_t _block_count;

public:
    device();
    ~device();

public:
    bool open(char const *device, size_t block_size = 0);
    void close();

public:
    inline size_t get_block_size() const
    { return _block_size; }
    inline uint64_t get_block_count() const
    { return _block_count; }

public:
    bool read(uint64_t lba, void *blocks, size_t count, size_t *nread) const;

public:
    template <typename T>
    inline T *new_block() const
    {
        size_t block_size = std::max(get_block_size(), sizeof(T));
        if (block_size == 0)
            return nullptr;

        return reinterpret_cast <T *> (new (std::nothrow) uint8_t[block_size]);
    }

    template <typename T>
    static inline void free_block(T *&ptr)
    {
        if (ptr != nullptr) {
            delete[] reinterpret_cast<uint8_t *> (ptr);
            ptr = nullptr;
        }
    }

public:
    template <typename T>
    inline bool read(uint64_t lba, T *object, bool validate = true) const
    {
        if (!read(lba, object, 1, nullptr))
            return false;

        if (validate && !::nx_object_verify(reinterpret_cast <nx_object_t *> (object))) {
            errno = ENOTBLK;
            return false;
        }

        return true;
    }
};

}

#endif  // !__nx_device_h
