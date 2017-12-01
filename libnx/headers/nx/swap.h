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

#ifndef __nx_swap_h
#define __nx_swap_h

#include "nx/format/swap.h"

#include <cstdint>

namespace nx {
    template <typename T>
    static inline T swap(T const &x) { return x; }
    template <>
    inline int16_t swap(int16_t const &x) { return nx_swap16(x); }
    template <>
    inline uint16_t swap(uint16_t const &x) { return nx_swap16(x); }
    template <>
    inline int32_t swap(int32_t const &x) { return nx_swap32(x); }
    template <>
    inline uint32_t swap(uint32_t const &x) { return nx_swap32(x); }
    template <>
    inline int64_t swap(int64_t const &x) { return nx_swap64(x); }
    template <>
    inline uint64_t swap(uint64_t const &x) { return nx_swap64(x); }
}

#endif  // !__nx_swap_h
