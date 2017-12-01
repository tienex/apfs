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

#include "apfs/internal/xattr.h"

#include <cstring>

using apfs::internal::xattr;

xattr::xattr()
    : object()
{
}

void xattr::
set_inline_content(byte_vector &&content)
{
    _content = std::move(content);
}

uint64_t xattr::
get_size() const
{
    return (!_content.empty() ? _content.size() : object::get_size());
}

uint64_t xattr::
get_alloced_size() const
{
    return (!_content.empty() ? _content.size() : object::get_alloced_size());
}

ssize_t xattr::
read(nx::device *device, void *buf, size_t size, nx_off_t offset) const
{
    if (size == 0)
        return 0;

    if (_content.empty())
        return object::read(device, buf, size, offset);

    size_t content_size = _content.size();
    uint8_t *bytes = reinterpret_cast<uint8_t *>(buf);

    if (offset < 0 || offset >= content_size)
        return 0;

    if (offset + size >= content_size) {
        size = content_size - offset;
        if (size == 0)
            return 0;
    }

    memcpy(buf, &_content[offset], size);
    return size;
}
