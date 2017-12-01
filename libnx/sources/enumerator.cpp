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

#include "nx/enumerator.h"
#include "nx/device.h"

#include "nxcompat/nxcompat.h"

using nx::enumerator;

enumerator::enumerator(object *owner, device *device, uint64_t root_lba,
        uint64_t oid, uint32_t tree_type, oid_mapper_type const &mapper,
        oid_comparer_type const &comparer)
    : _oid      (oid)
    , _root_lba (root_lba)
    , _tree_type(tree_type)
    , _owner    (owner)
    , _device   (device)
    , _root     (nullptr)
    , _stack    ([](node_type &node) { device::free_block(node.first); })
    , _mapper   (mapper)
    , _compare  (comparer)
    , _end      (false)
{
    if (!_compare) {
        _compare = [](uint64_t a, uint64_t b)
        { return (a > b) ? 1 : (a < b) ? -1 : 0; };
    }
}

enumerator::~enumerator()
{
}

bool enumerator::
check_node(nx_btn_t const *node) const
{
    auto context = _owner->get_context();
    auto tree_type = (node->btn_level == _root->btn_level ?
            _tree_type : NX_OBJECT_TYPE_BTREE_NODE);

    if (tree_type != 0 &&
            !(nx::swap(node->btn_o.o_type) == tree_type ||
              NX_OBJECT_GET_TYPE(nx::swap(node->btn_o.o_type)) == tree_type)) {
        context->log(severity::error,
                "expected btree node of type %#" PRIx64 ", "
                "got a btree node of type %#" PRIx64,
                tree_type, nx::swap(node->btn_o.o_type));
        return false;
    }
    if (node != _root && nx::swap(node->btn_level) >
            nx::swap(_root->btn_level)) {
        context->log(severity::error,
                "btree node level #%u is greater than btree root node "
                "level %#u", nx::swap(node->btn_level),
                nx::swap(_root->btn_level));
        return false;
    }

    return true;
}

bool enumerator::
fetch_node_kv(nx_btn_t const *node, size_t index, btn_kvinfo_t *kvi,
        void const **key, void const **value) const
{
    auto context = _owner->get_context();
    if (!::nx_btn_get_kvinfo(node, _root, index, kvi)) {
        context->log(severity::error,
                "failed retrieving key/value information in "
                "btree node slot #%" PRIuSIZE, index);
        return false;
    }
    if (!::nx_btn_get_kvptrs(node, _root, kvi, key, value)) {
        context->log(severity::error,
                "failed retrieving key/value pointers in "
                "btree node slot #%" PRIuSIZE, index);
        return false;
    }

    return true;
}

bool enumerator::
is_index_node_valid(btn_kvinfo_t const &kvi) const
{
    auto context = _owner->get_context();
    if (kvi.val_size != sizeof(uint64_t)) {
        context->log(severity::error,
                "expected value to be of length %" PRIuSIZE ", "
                "got a value of length %" PRIuSIZE,
                sizeof(uint64_t), kvi.val_size);
        return false;
    }
    return true;
}

bool enumerator::
reset()
{
    auto context = _owner->get_context();

    _stack.clear();

    //
    // Fetch the root of the tree.
    //
    if (!_owner->read_btn(_device, _root_lba, _root))
        return false;

    //
    // Now traverse the tree to the first leaf matching oid.
    //

    nx_btn_t *node;
    nx_btn_t *next_node;
    size_t    index = 0;
    for (node = _root;;) {
        //
        // Ensure it's maching the specs.
        //
        if (!check_node(node))
            goto fail;

        uint64_t const *key;
        uint64_t const *val;
        uint64_t const *next_key;
        uint64_t const *next_val;

        for (size_t n = index; n < nx::swap(node->btn_nkeys); n++) {
            btn_kvinfo_t kvi;
            uint64_t     oid;
            uint64_t     next_oid = INT64_MAX;
            bool         at_eot   = (n + 1 >= nx::swap(node->btn_nkeys));

            if (!fetch_node_kv(node, n, &kvi,
                        reinterpret_cast<void const **>(&key),
                        reinterpret_cast<void const **>(&val)))
                goto fail;

            oid = nx::swap(*key);

            //
            // Quick reject.
            //
            if (_compare(oid, _oid) > 0)
                goto fail;

            if (!at_eot) {
                if (!fetch_node_kv(node, n + 1, &kvi,
                            reinterpret_cast<void const **>(&next_key),
                            reinterpret_cast<void const **>(&next_val)))
                    goto fail;

                next_oid = nx::swap(*next_key);
            }

            if (node->btn_level == 0) {
                //
                // If the node level is zero, we have leaves, as such we
                // need to find the value.
                //

                if (_compare(nx::swap(*key), _oid) == 0) {
                    //
                    // This node is the TOS.
                    //
                    _stack.push(std::make_pair(node, n));
                    goto success;
                }
            } else {
                uint64_t lba;

                //
                // Ensure it's a valid index node.
                //
                if (!is_index_node_valid(kvi))
                    goto fail;

                //
                // The key must be oid <= key <= next_oid.
                //
                if (_compare(oid, _oid) <= 0 && _compare(_oid, next_oid) <= 0) {
                    //
                    // Save the node and the index.
                    //
                    _stack.push(std::make_pair(node, n));

                    //
                    // Fetch this node.
                    //
                    lba = nx::swap(*val);
                    if (_mapper && !_mapper(lba, lba))
                        goto fail;

                    if (!_owner->read_btn(_device, lba, node))
                        goto fail;

                    //
                    // We must reset index.
                    //
                    n = -1;
                }
            }
        }

        //
        // Retry from the previous node, from the next entry.
        //
        if (_stack.empty())
            goto fail;

        node  = _stack.top().first;
        index = _stack.top().second + 1;
        _stack.pop(false);
    }

success:
    _end = false;
    return true;

fail:
    if (node->btn_level == 0) {
        context->log(severity::error, "cannot find oid %" PRIu64 " in btree",
                _oid);
    } else {
        context->log(severity::error, "oid %" PRIu64 " is smaller than the "
                "smallest key in btree", _oid);
    }

    if (_stack.empty()) {
        device::free_block(_root);
        _root = nullptr;
    }
    _stack.clear();

    _end = true;
    return false;
}

bool enumerator::
advance()
{
    nx_btn_t *node = nullptr;

    for (;;) {
        //
        // To advance we pop this node, and read the following.
        //
        if (_stack.depth() == 1) {
            //
            // No more entries to pop!
            //
            return false;
        }

        _stack.pop();

        //
        // Now increment the index.
        //
        auto  node  = _stack.top().first;
        auto &index = _stack.top().second;

        //
        // If the index is past the number of keys of this node, we must read
        // the following, or traverse upward to the next.
        //
        if (++index >= nx::swap(node->btn_nkeys))
            continue;

        //
        // Fetch key and value for the new index.
        //
        btn_kvinfo_t    kvi;
        uint64_t const *key;
        uint64_t const *val;

        if (!fetch_node_kv(node, index, &kvi,
                    reinterpret_cast<void const **>(&key),
                    reinterpret_cast<void const **>(&val)))
            return false;

        //
        // Ensure the key still matches the requested oid.
        // If it does not, we're at the end of the enumeration.
        //
        if (_compare(nx::swap(*key), _oid) != 0)
            return false;

        //
        // Read the new node, traversing downward if level != 0, here it's
        // easy because a continuation must always start at zero, so we
        // don't have to iterate through each key.
        //
        if (node->btn_level != 0) {
            do {
                //
                // Ensure it's a valid index node.
                //
                if (!is_index_node_valid(kvi))
                    return false;

                //
                // Read the next btree node.
                //
                uint64_t oid = nx::swap(*val), lba = oid;
                if (_mapper && !_mapper(oid, lba))
                    return false;

                if (!_owner->read_btn(_device, lba, node))
                    return false;

                //
                // Ensure it's maching the specs.
                //
                if (!check_node(node))
                    goto fail;

                //
                // Fetch key and value for the new node.
                //
                if (!fetch_node_kv(node, 0, &kvi,
                            reinterpret_cast<void const **>(&key),
                            reinterpret_cast<void const **>(&val)))
                    goto fail;

                //
                // Let's ensure that the first key matches the oid we need.
                //
                if (_compare(nx::swap(*key), _oid) != 0)
                    return false;

                //
                // Save this node onto the stack.
                //
                _stack.push(std::make_pair(node, 0));
            } while (node->btn_level != 0);
        }

        break;
    }

    //
    // Success!
    //
    return true;

fail:
    //
    // Release this node because we're not pushing it onto
    // the stack.
    //
    device::free_block(node);
    return false;
}

bool enumerator::
next(object::sized_value_type &key, object::sized_value_type &value)
{
    if (_end || _stack.empty())
        return false;

    //
    // Obtain the values.
    //
    auto  node    = _stack.top().first;
    auto &index   = _stack.top().second;

    //
    // If the index is past the number of keys of this node, we must read
    // the following, or traverse upward to the next.
    //
    if (index >= nx::swap(node->btn_nkeys)) {
        if (!advance()) {
            _end = true;
            return false;
        }

        // Refetch.
        node  = _stack.top().first;
        index = _stack.top().second;
    }

    btn_kvinfo_t kvi;
    if (!fetch_node_kv(node, index, &kvi,
                reinterpret_cast<void const **>(&key.first),
                reinterpret_cast<void const **>(&value.first))) {
        _end = true;
        return false;
    }

    //
    // Check this node matches oid, if not we're at the end of the enumeration.
    //
    if (_compare(nx::swap(*reinterpret_cast<uint64_t const *>(key.first)),
                _oid) != 0) {
        _end = true;
        return false;
    }

    key.second = kvi.key_size;
    value.second = kvi.val_size;

    //
    // Increment index, the next call will eventually traverse the tree.
    //
    index++;

    return true;
}
