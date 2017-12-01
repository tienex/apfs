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

#ifndef __apfs_internal_directory_h
#define __apfs_internal_directory_h

#include "apfs/internal/base.h"

namespace apfs { namespace internal {

class directory {
public:
    struct entry {
        using name_map = std::map<std::string, entry>;

        uint64_t    oid;
        uint64_t    timestamp;
        uint32_t    hash;
        std::string name;

        inline bool check_hash() const
        {  return (hash == ::apfs_hash_name(&name[0], name.length(), true)); }
    };

protected:
    entry::name_map _entries;

protected:
    void add_entry(void const *k, void const *v, bool insensitive);

protected:
    inline entry::name_map const &get_entries() const
    { return _entries; }
};

} }

#endif  // !__apfs_internal_directory_h
