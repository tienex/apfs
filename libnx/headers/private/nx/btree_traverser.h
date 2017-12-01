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

#ifndef __nx_btree_traverser_h
#define __nx_btree_traverser_h

#include "nx/object.h"
#include "nx/stack.h"

namespace nx {

class btree_traverser {
public:
    typedef std::function<bool(uint64_t, uint64_t &)> oid_mapper_type;

private:
    typedef std::pair<nx_btn_t *, size_t> btn_chidx;

    object                         *_owner;
    device                         *_device;
    nx_btn_t                       *_root;
    stack<btn_chidx>                _stack;
    object::generic_traverser_type  _callback;
    oid_mapper_type                 _mapper;
    uint32_t                        _tree_type;

public:
    btree_traverser(object *owner, device *device, nx_btn_t *root,
            object::generic_traverser_type const &callback,
            uint32_t tree_type = 0,
            oid_mapper_type const &mapper = oid_mapper_type());

public:
    void traverse();

private:
    static bool static_callback(void *opaque, void const *key, size_t key_size,
            void const *val, size_t val_size);

private:
    inline bool iterate(object::sized_value_type const &key,
            object::sized_value_type const &val);
};

}

#endif  // !__nx_btree_traverser_h
