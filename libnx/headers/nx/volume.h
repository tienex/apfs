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

#ifndef __nx_volume_h
#define __nx_volume_h

#include "nx/container.h"
#include "nx/format/apfs.h"

namespace nx {

class enumerator;

class volume : public object {
private:
    container *_owner;
    apfs_fs_t *_super;

protected:
    friend class container;
    volume(context *context, container *owner);

public:
    virtual ~volume();

protected:
    bool open(uint64_t lba);

public:
    inline bool is_case_sensitive() const
    { return (nx::swap(_super->apfs_incompatible_features) &
            APFS_FS_INCOMPATIBLE_FEATURE_CASE_SENSITIVE) == 0; }

public:
    inline nx_uuid_t const &get_uuid() const
    { return _super->apfs_vol_uuid; }
    inline char const *get_name() const
    { return _super->apfs_volname; }
    inline size_t get_block_size() const
    { return _owner->get_block_size(); }
    inline uint64_t get_used_block_count() const
    { return nx::swap(_super->apfs_fs_alloc_count); }
    inline uint64_t get_reserved_block_count() const
    { return nx::swap(_super->apfs_fs_reserve_block_count); }
    inline uint64_t get_free_block_count(bool root) const
    { return _owner->get_block_count() - (get_used_block_count() +
            (root ? 0 : get_reserved_block_count())); }
    inline uint64_t get_free_block_count() const
    { return get_free_block_count(true); }
    inline uint64_t get_available_block_count() const
    { return get_free_block_count(false); }

public:
    uint64_t get_inode_used_count() const;

public:
    enumerator *open_oid(uint64_t oid) const;
    enumerator *open_root() const;

private:
    bool read_super(device *device, uint64_t lba, apfs_fs_t *&super);

private:
    inline bool lookup_omap_oid(device *device, uint64_t oid, uint32_t type,
            uint64_t &paddr, uint64_t &size) const
    {
        return object::lookup_omap_oid(device, nx::swap(_super->apfs_omap_oid),
                oid, type, paddr, size);
    }

public:
    void traverse_omap(omap_traverser_type const &callback) const;
    inline void traverse_omap(bool (*callback)(void *, uint32_t, uint32_t,
                uint32_t, nx_omap_key_t const &, nx_omap_value_t const &),
            void *opaque)
    {
        if (callback == nullptr)
            return;

        traverse_omap([=](uint32_t max_level, uint32_t level, uint32_t index,
                    nx_omap_key_t const &key, nx_omap_value_t const &value)
                { return (*callback)(opaque, max_level, level, index, key,
                        value); });
    }

    void traverse_root(generic_traverser_type const &callback) const;
    inline void traverse_root(bool (*callback)(void *, uint32_t, uint32_t,
                uint32_t, nx::object::sized_value_type const &,
                nx::object::sized_value_type const &),
            void *opaque)
    {
        if (callback == nullptr)
            return;

        traverse_root([=](uint32_t max_level, uint32_t level, uint32_t index,
                    nx::object::sized_value_type const &key,
                    nx::object::sized_value_type const &value)
                { return (*callback)(opaque, max_level, level, index, key,
                        value); });
    }

private:
    inline uint64_t get_root_tree_lba() const;
};

}

#endif  // !__nx_volume_h
