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

#include "nx/object.h"
#include "nx/enumerator.h"

#include "nxcompat/nxcompat.h"

#include <cstdlib>

using nx::object;

object::object(nx::context *context)
    : _context    (context)
{
    assert(_context != nullptr && "context cannot be null");
    if (_context == nullptr) {
        std::abort();
    }
}

object::~object()
{
}

bool object::
read_omap(device *device, uint64_t lba, nx_omap_t *&omap) const
{
    omap = device->new_block <nx_omap_t> ();
    if (omap == nullptr) {
        _context->log(severity::fatal, "not enough memory to allocate "
                "object map");
        return false;
    }

    if (!device->read(lba, omap, false)) {
        _context->log(severity::fatal, "cannot read object map at lba "
                "%" PRIu64 ": %s", lba, ::strerror(errno));
        nx::device::free_block(omap);
        return false;
    }

    if (nx::swap(omap->om_o.o_type) != NX_OBJECT_DIRECT_TYPE(OBJECT_MAP)) {
        _context->log(severity::error, "block %" PRIu64 " is not an "
                "object map block", lba);
        nx::device::free_block(omap);
        return false;
    }

    if (!::nx_object_verify(&omap->om_o)) {
        _context->log(severity::error, "object map verification failed, "
                "checksum mismatch (expected %#" PRIx64 ", got %#"
                PRIx64 ")", nx::swap(omap->om_o.o_checksum),
                ::nx_object_checksum(&omap->om_o));
        nx::device::free_block(omap);
        return false;
    }

    return true;
}

bool object::
read_btn(device *device, uint64_t lba, nx_btn_t *&btn) const
{
    btn = device->new_block <nx_btn_t> ();
    if (btn == nullptr) {
        _context->log(severity::fatal, "not enough memory to allocate "
                "btree node");
        return false;
    }

    if (!device->read(lba, btn, false)) {
        _context->log(severity::fatal, "cannot read btree node at lba "
                "%" PRIu64 ": %s", lba, ::strerror(errno));
        nx::device::free_block(btn);
        return false;
    }

    if (NX_OBJECT_GET_TYPE(nx::swap(btn->btn_o.o_type)) != NX_OBJECT_TYPE_BTREE_ROOT &&
        NX_OBJECT_GET_TYPE(nx::swap(btn->btn_o.o_type)) != NX_OBJECT_TYPE_BTREE_NODE) {
        _context->log(severity::error, "block %" PRIu64 " is not a "
                "btree node block", lba);
        nx::device::free_block(btn);
        return false;
    }

    if (!::nx_object_verify(&btn->btn_o)) {
        _context->log(severity::error, "btree node verification failed, "
                "checksum mismatch (expected %#" PRIx64 ", got %#"
                PRIx64 ")", nx::swap(btn->btn_o.o_checksum),
                ::nx_object_checksum(&btn->btn_o));
        nx::device::free_block(btn);
        return false;
    }

    return true;
}

bool object::
lookup_omap_oid(device *device, uint64_t omap_oid, uint64_t oid,
        uint32_t type, uint64_t &paddr, uint64_t &size) const
{
    nx_omap_t *omap;

    if (!read_omap(device, omap_oid, omap))
        return false;

    bool result = lookup_omap_oid(device, omap, oid, type, paddr, size);

    device::free_block(omap);

    return result;
}

bool object::
lookup_omap_oid(device *device, nx_omap_t const *omap, uint64_t oid,
        uint32_t type, uint64_t &paddr, uint64_t &size) const
{
    enumerator e(const_cast<object *>(this), device,
            nx::swap(omap->om_tree_oid), oid, nx::swap(omap->om_tree_type));
    if (!e.reset())
        return false;

    sized_value_type k, v;
    if (!e.next(k, v))
        return false;

    auto value = reinterpret_cast<nx_omap_value_t const *>(v.first);
    paddr = nx::swap(value->ov_oid);
    size = device->get_block_size();

    return true;
}
