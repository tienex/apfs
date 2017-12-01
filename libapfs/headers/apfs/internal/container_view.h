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

#ifndef __apfs_internal_container_view_h
#define __apfs_internal_container_view_h

#include <iterator>

namespace apfs { namespace internal {

template <typename T>
class container_view {
private:
    typedef typename T::const_iterator Iterator;
    typedef container_view<T> Self;

private:
    Iterator _begin;
    Iterator _end;

public:
    // ...
    container_view(T const &container)
        : container_view(container.begin(), container.end())
    { }

    container_view(Iterator begin, Iterator end)
        : _begin(begin), _end(end)
    { }

public:
    inline Iterator begin() const
    { return this->_begin; }
    inline Iterator end() const
    { return this->_end; }

public:
    inline size_t size() const
    { return this->_end - this->_begin; }
    inline bool empty() const
    { return (size() == 0); }

public:
    inline typename std::iterator_traits<Iterator>::reference
    operator[](std::size_t index) const
    { return this->_begin[index]; }

public:
    inline Self operator + (size_t start) const
    {
        if (this->_begin + start >= this->_end)
            return Self(this->_end, this->_end);
        else
            return Self(this->_begin + start, this->_end);
    }
};

} }

#endif  // !__apfs_internal_container_view_h
