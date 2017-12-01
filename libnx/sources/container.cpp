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

#include "nx/container.h"
#include "nx/volume.h"
#include "nx/btree_traverser.h"

#include "nxcompat/nxcompat.h"

#include <map>

using nx::container;
using nx::volume;

container::container(nx::context *context)
    : object      (context)
    , _main_super (nullptr)
    , _tier2_super(nullptr)
{
}

container::~container()
{
    close();
}

bool container::
open(bool last_xid)
{
    if (_main_super != nullptr) {
        _context->log(severity::error, "container is already opened");
        return false;
    }

    auto device = _context->get_main_device();
    if (device == nullptr) {
        _context->log(severity::fatal, "no main device associated to context");
        return false;
    }

    if (!read_super(device, 0, _main_super))
        return false;

    // Find the last checkpoint
    if (!find_tier2_super(device, last_xid ? UINT64_MAX : 0,
                false, _tier2_super)) {
        close();
        return false;
    }

    return true;
}

bool container::
open_at(uint64_t xid)
{
    if (_main_super != nullptr) {
        _context->log(severity::error, "container is already opened");
        return false;
    }

    auto device = _context->get_main_device();
    if (device == nullptr) {
        _context->log(severity::fatal, "no main device associated to context");
        return false;
    }

    if (!read_super(device, 0, _main_super))
        return false;

    // Find the last checkpoint
    if (!find_tier2_super(device, xid, true, _tier2_super)) {
        close();
        return false;
    }

    return true;
}

void container::
close()
{
    if (_tier2_super != _main_super) {
        nx::device::free_block(_tier2_super);
    } else {
        _tier2_super = nullptr;
    }

    nx::device::free_block(_main_super);
}

bool container::
read_super(device *device, uint64_t lba, nx_super_t *&super, bool quiet)
{
    super = device->new_block <nx_super_t> ();
    if (super == nullptr) {
        _context->log(severity::fatal, "not enough memory to allocate "
                "nx super");
        return false;
    }

    if (!device->read(lba, super, false)) {
        _context->log(severity::fatal, "cannot read nx super at lba "
                "%" PRIu64 ": %s", lba, ::strerror(errno));
        nx::device::free_block(super);
        return false;
    }

    if (nx::swap(super->nx_o.o_type) != NX_OBJECT_CPMAP_TYPE(CONTAINER)) {
        if (!quiet) {
            _context->log(severity::error, "block %" PRIu64 " is not a nx "
                    "super block", lba);
        }
        nx::device::free_block(super);
        return false;
    }

    if (nx::swap(super->nx_signature) != NX_SUPER_SIGNATURE) {
        if (!quiet) {
            _context->log(severity::error, "block %" PRIu64 " has an invalid "
                    "nx super signature", lba);
        }
        nx::device::free_block(super);
        return false;
    }

    if (!::nx_object_verify(&super->nx_o)) {
        if (!quiet) {
            _context->log(severity::error, "nx super verification failed, "
                    "checksum mismatch (expected %#" PRIx64 ", got %#"
                    PRIx64 ")", nx::swap(super->nx_o.o_checksum),
                    ::nx_object_checksum(&super->nx_o));
        }
        nx::device::free_block(super);
        return false;
    }

    return true;
}

//
// https://stackoverflow.com/questions/1376498/is-there-a-nearest-key-map-datastructure
//
template <typename T, typename U>
static inline typename std::map<T, U>::iterator
find_nearest(std::map<T,U> &map, T const &key)
{
    auto itlow = map.lower_bound(key);
    auto itprev = itlow;

    if (itlow == map.begin())
        return itlow;

    itlow--;

    //
    // for cases when we have "key" element in our map
    // or "key" occures before the first element of map
    //
    if (itlow->first == key || itprev == map.begin())
        return itlow;

    //
    // if "item"to_find" is besides the last element of map
    //
    if (itlow == map.end())
        return itprev;

    return (itlow->first - key < key - itprev->first)
        ? itlow : itprev;
}

bool container::
find_tier2_super(device *device, uint64_t xid, bool equal, nx_super_t *&super)
{
    //
    // Read all super, we do so to allow broken apfs to be read from
    // any checkpoint.
    //
    uint64_t lba;
    uint64_t main_xid  = nx::swap(_main_super->nx_o.o_xid);
    uint64_t tier2_xid = main_xid;

    std::map<uint64_t, uint64_t> tier2_super;

    // Initially the tier2 super is the same to main.
    super = _main_super;

    // Insert the main super.
    tier2_super[nx::swap(super->nx_o.o_xid)] = 0;

    for (; nx::swap(super->nx_xp_desc_next) != 0;) {
        nx_super_t *new_super;

        //
        // Compute the next super in the checkpoint descriptors
        //
        if (super->nx_xp_desc_next == 0)
            break;

        uint32_t bidx = (nx::swap(super->nx_xp_desc_next) +
                nx::swap(super->nx_xp_desc_len) - 1);

        if (bidx >= nx::swap(super->nx_xp_desc_blocks))
            break;

        lba = nx::swap(super->nx_xp_desc_first) + bidx;

        if (!read_super(device, lba, new_super, true))
            break;

        // Release old super, if it's not the main.
        if (super != _main_super) {
            nx::device::free_block(super);
        }

        super = new_super;

        tier2_xid = nx::swap(super->nx_o.o_xid);
        if (tier2_xid >= main_xid) {
            _context->log(severity::warning, "found main xid %" PRIu64
                    " inferior to tier2 xid %" PRIu64 ", ignoring",
                    main_xid, tier2_xid);
        }

        tier2_super[tier2_xid] = lba;
    }

    if (super != _main_super) {
        nx::device::free_block(super);
    }

    //
    // Find the tier2 super.
    //
    std::map<uint64_t, uint64_t>::const_iterator i;

    if (equal) {
        i = tier2_super.find(xid);
    } else {
        i = find_nearest(tier2_super, xid);
    }

    if (i == tier2_super.end()) {
        _context->log(severity::error, "cannot find tier2 xid %" PRIu64, xid);
        return false;
    }

    tier2_xid = i->first;
    if (!read_super(device, i->second, super, true))
        return false;

    _context->log(severity::info, "selected checkpoint xid %" PRIu64,
            tier2_xid);

    return true;
}

bool container::
read_cpm(device *device, uint64_t lba, nx_cpm_t *&cpm) const
{
    cpm = device->new_block <nx_cpm_t> ();
    if (cpm == nullptr) {
        _context->log(severity::fatal, "not enough memory to allocate cpm");
        return false;
    }

    if (!device->read(lba, cpm, false)) {
        _context->log(severity::fatal, "cannot read cpm at lba "
                "%" PRIu64 ": %s", lba, ::strerror(errno));
        nx::device::free_block(cpm);
        return false;
    }

    if (nx::swap(cpm->cpm_o.o_type) != NX_OBJECT_DIRECT_TYPE(CHECKPOINT_MAP)) {
        _context->log(severity::error, "block %" PRIu64 " is not a nx "
                "cpm block", lba);
        nx::device::free_block(cpm);
        return false;
    }

    if (!::nx_object_verify(&cpm->cpm_o)) {
        _context->log(severity::error, "cpm verification failed, "
                "checksum mismatch (expected %#" PRIx64 ", got %#"
                PRIx64 ")", nx::swap(cpm->cpm_o.o_checksum),
                ::nx_object_checksum(&cpm->cpm_o));
        nx::device::free_block(cpm);
        return false;
    }

    return true;
}

bool container::
lookup_checkpoint_oid(device *device, nx_super_t const *sb,
        uint64_t oid, uint32_t type, uint64_t &paddr, uint64_t &size) const
{
    nx_cpm_t *cpm = nullptr;

    //
    // Objects in the checkpoint map have the type always marked with
    // NX_OBJECT_FLAG_CHECKPOINT, as such, we override the user type.
    //
    type &= ~NX_OBJECT_FLAG_DIRECT;
    type |= NX_OBJECT_FLAG_CHECKPOINT;

    if (nx::swap(_main_super->nx_xp_desc_len) <= 1) {
        _context->log(severity::error, "no checkpoint descriptors found");
        return false;
    }

    for (size_t index = 0; index < nx::swap(sb->nx_xp_desc_len) - 1; index++) {
        uint64_t  lba;

        //
        // nx_super.nx_xp_desc_blocks is a 31-bits value, if this value has
        // the highest bit set, then the block must be searched because
        // fragmented, we currently ignore this case but assert.
        //
        if ((nx::swap(sb->nx_xp_desc_blocks) & 0x80000000) != 0) {
            //
            // lookup block = (index + sb->nx_xp_desc_index) % sb->nx_desc_blocks.
            //
            _context->log(severity::fatal, "fragmented checkpoint descriptors "
                    "not supported");
            return false;
        } else {
            lba = (nx::swap(sb->nx_xp_desc_first) +
                   nx::swap(sb->nx_xp_desc_index) + index);
        }

        nx::device::free_block(cpm);
        if (!read_cpm(device, lba, cpm))
            return false;

        for (size_t n = 0; n < nx::swap(cpm->cpm_count); n++) {
            nx_cpm_entry_t const *entry = &cpm->cpm_map[n];

            if (nx::swap(entry->cpm_type) == type &&
                nx::swap(entry->cpm_oid) == oid) {
                paddr = nx::swap(entry->cpm_paddr);
                size = nx::swap(entry->cpm_size);
                nx::device::free_block(cpm);
                return true;
            }
        }
    }

    nx::device::free_block(cpm);
    _context->log(severity::warning, "cannot find oid %# " PRIx64 " type "
            "%#" PRIx32 " in checkpoint map", oid, type);

    return false;
}

volume *container::
open_volume(size_t index) const
{
    auto super = get_super();
    if (super == nullptr)
        return nullptr;

    if (index >= nx::swap(super->nx_max_file_systems) ||
        super->nx_fs_oid[index] == 0) {
        _context->log(severity::error, "invalid volume index %" PRIuSIZE
                " specified", index);
        return nullptr;
    }

    uint64_t paddr, size;
    if (!lookup_omap_oid(get_main_device(), super,
                nx::swap(super->nx_fs_oid[index]),
                NX_OBJECT_OMAP_TYPE(APFS_VOLUME),
                paddr, size))
        return nullptr;

    auto v = new (std::nothrow) volume(get_context(),
            const_cast <container *> (this));
    if (v == nullptr) {
        _context->log(severity::error, "not enough memory to allocate volume "
                "object");
        return nullptr;
    }

    if (!v->open(paddr)) {
        delete v;
        v = nullptr;
    }

    return v;
}

void container::
traverse_omap(omap_traverser_type const &callback)
{
    if (!callback)
        return;

    nx_omap_t *omap;
    nx_btn_t *btn;
    auto device = get_main_device();

   if (!read_omap(device, nx::swap(get_super()->nx_omap_oid), omap))
        return;

    if (!read_btn(device, nx::swap(omap->om_tree_oid), btn)) {
        device::free_block(omap);
        return;
    }

    btree_traverser(this, device, btn,
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

void container::
scavenge(std::function<bool(uint64_t, nx_object_t const *)> const &callback,
        uint64_t blockno) const
{
    bool stop = false;

    if (!callback)
        return;

    auto device = _context->get_main_device();

    auto object = device->new_block<nx_object_t>();
    if (object == nullptr)
        return;

    if (blockno == static_cast<uint64_t>(-1)) {
        blockno = 0;
    } else {
        stop = true;
    }

    for (;; blockno++) {
        if (!device->read(blockno, object, false))
            break;

        //
        // Skip invalid objects.
        //
        if (object->o_checksum == 0 || object->o_checksum == UINT64_MAX)
            continue;

        if (!::nx_object_verify(object))
            continue;

        if (!callback(blockno, object))
            break;

        if (stop)
            break;
    }
}

bool container::
get_info(info &info) const
{
    uint64_t paddr, size;
    auto     device = get_main_device();
    auto     super  = get_main_super();

    if (!lookup_checkpoint_oid(device, super, nx::swap(super->nx_spaceman_oid),
                NX_OBJECT_TYPE_SPACEMAN, paddr, size))
        return false;

    auto sm = device->new_block<nx_spaceman_t>();
    if (sm == nullptr)
        return false;

    if (!device->read(paddr, sm))
        return false;

    info.blksize = get_block_size();
    info.blocks  = get_block_count();
    info.bfree   = nx::swap(sm->sm_free_count);
    info.volumes = nx::swap(super->nx_max_file_systems);
    info.vfree   = 0;

    device::free_block(sm);

    for (size_t n = 0; n < info.volumes; n++) {
        if (super->nx_fs_oid[n] != 0) {
            info.vfree++;
        }
    }

    info.vfree   = info.volumes - info.vfree;

    return true;
}
