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

#include "nx/volume.h"
#include "nx/container.h"
#include "nx/enumerator.h"
#include "nx/btree_traverser.h"

#include "nxcompat/nxcompat.h"

using nx::volume;

volume::volume(context *context, container *owner)
    : object(context)
    , _owner(owner)
    , _super(nullptr)
{
}

volume::~volume()
{
    device::free_block(_super);
}

bool volume::
open(uint64_t lba)
{
    return read_super(_owner->get_main_device(), lba, _super);
}

bool volume::
read_super(device *device, uint64_t lba, apfs_fs_t *&super)
{
    super = device->new_block <apfs_fs_t> ();
    if (super == nullptr) {
        _context->log(severity::fatal, "not enough memory to allocate "
                "apfs super");
        return false;
    }

    if (!device->read(lba, super, false)) {
        _context->log(severity::fatal, "cannot read apfs super at lba "
                "%" PRIu64 ": %s", lba, ::strerror(errno));
        nx::device::free_block(super);
        return false;
    }

    if (nx::swap(super->apfs_o.o_type) != NX_OBJECT_OMAP_TYPE(APFS_VOLUME)) {
        _context->log(severity::error, "block %" PRIu64 " is not an apfs "
                "super block", lba);
        nx::device::free_block(super);
        return false;
    }

    if (nx::swap(super->apfs_signature) != APFS_FS_SIGNATURE) {
        _context->log(severity::error, "block %" PRIu64 " has an invalid "
                "apfs super signature", lba);
        nx::device::free_block(super);
        return false;
    }

    if (!::nx_object_verify(&super->apfs_o)) {
        _context->log(severity::error, "apfs super verification failed, "
                "checksum mismatch (expected %#" PRIx64 ", got %#"
                PRIx64 ")", nx::swap(super->apfs_o.o_checksum),
                ::nx_object_checksum(&super->apfs_o));
        nx::device::free_block(super);
        return false;
    }

    return true;
}

void volume::
traverse_omap(omap_traverser_type const &callback) const
{
    if (!callback)
        return;

    nx_omap_t *omap;
    nx_btn_t *btn;
    auto device = get_main_device();

    if (!read_omap(device, nx::swap(_super->apfs_omap_oid), omap))
        return;

    if (!read_btn(device, nx::swap(omap->om_tree_oid), btn)) {
        device::free_block(omap);
        return;
    }

    btree_traverser(const_cast<volume *>(this), device, btn,
            [&callback](uint32_t max_level, uint32_t level, uint32_t index,
                        sized_value_type const &key,
                        sized_value_type const &value)
            {
                return callback(max_level, level, index,
                        *reinterpret_cast<nx_omap_key_t const *>(key.first),
                        *reinterpret_cast<nx_omap_value_t const *>(value.first));
            },
            nx::swap(omap->om_tree_type)).traverse();

    device::free_block(omap);
}

inline uint64_t volume::
get_root_tree_lba() const
{
    uint64_t lba, size;

    if (!lookup_omap_oid(get_main_device(),
                nx::swap(_super->apfs_root_tree_oid),
                NX_OBJECT_OMAP_TYPE(BTREE_NODE),
                lba, size)) {
        lba = 0;
    }

    return lba;
}

uint64_t volume::
get_inode_used_count() const
{
    //
    // Open the root tree and get the value count.
    //
    uint64_t lba = get_root_tree_lba();
    if (lba == 0)
        return UINT64_MAX;

    nx_btn_t *root;
    if (!read_btn(get_main_device(), lba, root))
        return UINT64_MAX;

    bt_fixed_t *bt = NX_BTN_FIXED(root);
    uint64_t ninodes = nx::swap(bt->bt_key_count);

    device::free_block(root);

    return ninodes;
}

nx::enumerator *volume::
open_oid(uint64_t oid) const
{
    if (oid < 2)
        return nullptr;

    auto lba = get_root_tree_lba();
    if (lba == 0)
        return nullptr;

    auto device = get_main_device();
    auto e = new enumerator(const_cast<volume *>(this), device, lba,
            oid, NX_OBJECT_OMAP_TYPE(BTREE_ROOT),
            [=](uint64_t oid, uint64_t &lba)
            {
                uint64_t size;
                return lookup_omap_oid(device, oid, 0, lba, size);
            },
            [](uint64_t a, uint64_t b)
            {
                a = APFS_OBJECT_ID_ID(a);
                b = APFS_OBJECT_ID_ID(b);
                return (a > b) ? 1 : (a < b) ? -1 : 0;
            });

    if (e != nullptr && !e->reset()) {
        delete e;
        e = nullptr;
    }

    return e;
}

nx::enumerator *volume::
open_root() const
{
    return open_oid(APFS_DREC_ROOT_FILE_ID);
}

void volume::
traverse_root(generic_traverser_type const &callback) const
{
    if (!callback)
        return;

    auto lba = get_root_tree_lba();
    if (lba == 0)
        return;

    nx_btn_t *btn;
    auto device = get_main_device();
    if (!read_btn(device, lba, btn))
        return;

    btree_traverser(const_cast<volume *>(this), device, btn, callback, 0,
            [=](uint64_t oid, uint64_t &lba)
            {
                uint64_t size;
                return lookup_omap_oid(device, oid, 0, lba, size);
            }).traverse();
}
