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

#include "apfs/internal/directory.h"

#include "nxtools/string.h"

using apfs::internal::directory;

void directory::
add_entry(void const *k, void const *v, bool insensitive)
{
    auto dk = reinterpret_cast<apfs_drec_key_t const *>(k);
    auto dv = reinterpret_cast<apfs_drec_value_t const *>(v);

    auto name = std::string(dk->hashed.name,
            APFS_DREC_HASHED_NAME_LENGTH(dk) - 1);
    auto key = name;
    if (insensitive) {
        key = nxtools::to_lower(key);
    }

    _entries[key] =
        {
            .oid       = nx::swap(dv->file_id),
            .timestamp = nx::swap(dv->timestamp),
            .hash      = APFS_DREC_HASHED_NAME_HASH(dk),
            .name      = name
        };
}
