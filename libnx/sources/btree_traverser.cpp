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

#include "nx/btree_traverser.h"

#include "nxcompat/nxcompat.h"

using nx::btree_traverser;

btree_traverser::btree_traverser(object *owner, device *device, nx_btn_t *root,
        object::generic_traverser_type const &callback, uint32_t tree_type,
        oid_mapper_type const &mapper)
    : _owner    (owner)
    , _device   (device)
    , _root     (root)
    , _stack    ([](btn_chidx &x) { device::free_block(x.first); })
    , _callback (callback)
    , _mapper   (mapper)
    , _tree_type(tree_type)
{
}

void btree_traverser::
traverse()
{
    //
    // Starting from the root, call nx.c utility function.
    //
    _stack.push(std::make_pair(_root, 0));
    ::nx_btn_traverse(_root, _root, static_callback, this);
    _stack.clear();
}

bool btree_traverser::
static_callback(void *opaque, void const *key, size_t key_size,
        void const *val, size_t val_size)
{
    btree_traverser *self = reinterpret_cast<btree_traverser *>(opaque);
    return self->iterate(std::make_pair(key, key_size),
            std::make_pair(val, val_size));
}

inline bool btree_traverser::
iterate(object::sized_value_type const &key,
        object::sized_value_type const &val)
{
    bool  result    = true;
    auto  context   = _owner->get_context();
    auto &current   = _stack.top();
    auto  level     = nx::swap(current.first->btn_level);
    auto  tree_type = (level == nx::swap(_root->btn_level) ?
                       _tree_type : NX_OBJECT_TYPE_BTREE_NODE);

    if (_tree_type != 0 &&
            !(nx::swap(current.first->btn_o.o_type) == tree_type ||
              NX_OBJECT_GET_TYPE(nx::swap(current.first->btn_o.o_type)) == tree_type)) {
        context->log(severity::error,
                "expected btree node of type %#" PRIx64 ", "
                "got a btree node of type %#" PRIx64,
                tree_type,
                nx::swap(current.first->btn_o.o_type));
        return false;
    }

    if (val.second != 0 && level != 0 && val.second != sizeof(uint64_t)) {
        context->log(severity::error,
                "expected value to be of length %" PRIuSIZE ", "
                "got a value of length %" PRIuSIZE,
                sizeof(uint64_t), val.second);
        return false;
    }

    if (!_callback(nx::swap(_root->btn_level), level,
                current.second, key, val))
        return false;

    if (level != 0) {
        //
        // Traverse sub-tree
        //
        nx_btn_t *btn;
        uint64_t  lba;
        uint64_t  oid = nx::swap(*reinterpret_cast<uint64_t const *>(val.first));

        if (_mapper) {
            if (!_mapper(oid, lba))
                return false;
        } else {
            lba = oid;
        }

        if (!_owner->read_btn(_device, lba, btn))
            return false;

        _stack.push(std::make_pair(btn, 0));
        result = ::nx_btn_traverse(btn, _root, static_callback, this);
        _stack.pop();
    }

    current.second++;

    return result;
}
