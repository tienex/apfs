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

#ifndef __nx_object_h
#define __nx_object_h

#include "nx/context.h"
#include "nx/swap.h"

#include <functional>

namespace nx {

class object {
public:
    typedef std::pair<void const *, size_t> sized_value_type;
    typedef std::function<bool(uint32_t, uint32_t, uint32_t,
            sized_value_type const &,
            sized_value_type const &)> generic_traverser_type;

public:
    typedef std::function<bool(uint32_t, uint32_t, uint32_t,
            nx_omap_key_t const &,
            nx_omap_value_t const &)> omap_traverser_type;

protected:
    context *_context;

public:
    object(context *context);
    virtual ~object();

public:
    inline context *get_context() const
    { return _context; }

public:
    inline device *get_main_device() const
    { return _context->get_main_device(); }

protected:
    friend class btree_traverser;
    friend class enumerator;
    friend class volume;
    bool read_omap(device *device, uint64_t lba, nx_omap_t *&omap) const;
    bool read_btn(device *device, uint64_t lba, nx_btn_t *&btn) const;

protected:
    bool lookup_omap_oid(device *device, uint64_t omap_oid, uint64_t oid,
            uint32_t type, uint64_t &paddr, uint64_t &size) const;
    bool lookup_omap_oid(device *device, nx_omap_t const *omap, uint64_t oid,
            uint32_t type, uint64_t &paddr, uint64_t &size) const;
};

}

#endif  // !__nx_object_h
