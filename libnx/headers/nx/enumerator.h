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

#ifndef __nx_enumerator_h
#define __nx_enumerator_h

#include "nx/object.h"
#include "nx/stack.h"

namespace nx {

class device;

class enumerator {
public:
    typedef std::function<bool(uint64_t, uint64_t &)> oid_mapper_type;
    typedef std::function<int(uint64_t, uint64_t)> oid_comparer_type;

private:
    typedef std::pair<nx_btn_t *, size_t> node_type;

private:
    uint64_t           _oid;
    uint64_t           _root_lba;
    uint32_t           _tree_type;
    object            *_owner;
    device            *_device;
    nx_btn_t          *_root;
    stack<node_type>   _stack;
    oid_mapper_type    _mapper;
    oid_comparer_type  _compare;
    bool               _end;

protected:
    friend class object;
    friend class container;
    friend class volume;
    enumerator(object *owner, device *device, uint64_t root_lba,
            uint64_t oid, uint32_t tree_type = 0,
            oid_mapper_type const &mapper = oid_mapper_type(),
            oid_comparer_type const &comprarer = oid_comparer_type());

public:
    ~enumerator();

public:
    bool reset();
    bool next(object::sized_value_type &key, object::sized_value_type &value);

private:
    bool advance();
    bool check_node(nx_btn_t const *node) const;
    bool fetch_node_kv(nx_btn_t const *node, size_t index, btn_kvinfo_t *kvi,
            void const **key, void const **value) const;
    bool is_index_node_valid(btn_kvinfo_t const &kvi) const;
};

}

#endif  // !__nx_enumerator_h
