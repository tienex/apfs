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

#ifndef __nx_container_h
#define __nx_container_h

#include "nx/object.h"

namespace nx {

class volume;

class container : public object {
private:
    nx_super_t *_main_super;
    nx_super_t *_tier2_super;

public:
    struct info {
        uint32_t blksize;
        uint64_t blocks;
        uint64_t bfree;
        uint32_t volumes;
        uint32_t vfree;

        info()
        {
            blksize = 0;
            blocks = 0;
            bfree = 0;
            volumes = 0;
            vfree = 0;
        }
    };

public:
    container(nx::context *context);
    ~container();

public:
    bool open(bool last_xid = true);
    bool open_at(uint64_t xid);
    void close();

    inline bool is_open() const
    { return (_main_super != nullptr); }

public:
    inline nx_super_t const *get_main_super() const
    { return _main_super; }
    inline nx_super_t const *get_super() const
    { return _tier2_super; }

public:
    bool get_info(info &info) const;
    inline size_t get_block_size() const
    { return nx::swap(get_super()->nx_block_size); }
    inline uint64_t get_block_count() const
    { return nx::swap(get_super()->nx_block_count); }

public:
    volume *open_volume(size_t index) const;

public:
    inline nx_uuid_t const &get_uuid() const
    { return get_super()->nx_uuid; }

private:
    bool read_super(device *device, uint64_t lba, nx_super_t *&super,
            bool quiet = false);
    bool find_tier2_super(device *device, uint64_t xid, bool equal,
            nx_super_t *&super);

private:
    bool read_cpm(device *device, uint64_t lba, nx_cpm_t *&cpm) const;

private:
    bool lookup_checkpoint_oid(device *device, nx_super_t const *sb,
            uint64_t oid, uint32_t type, uint64_t &paddr, uint64_t &size) const;
    inline bool lookup_omap_oid(device *device, nx_super_t const *sb,
            uint64_t oid, uint32_t type, uint64_t &paddr, uint64_t &size) const
    {
        return object::lookup_omap_oid(device, nx::swap(sb->nx_omap_oid), oid,
                type, paddr, size);
    }

public:
    void traverse_omap(omap_traverser_type const &callback);
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

    void scavenge(std::function<bool(uint64_t,
                nx_object_t const *)> const &callback,
            uint64_t blockno = static_cast<uint64_t>(-1)) const;
    inline void scavenge(bool (*callback)(void *, uint64_t, nx_object_t const *),
            void *opaque, uint64_t blockno = static_cast<uint64_t>(-1)) const
    {
        if (callback == nullptr)
            return;

        scavenge([=](uint64_t bno, nx_object_t const *object)
                { return (*callback)(opaque, bno, object); }, blockno);
    }
};

}

#endif  // !__nx_container_h
