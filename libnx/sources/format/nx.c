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

#include "nx/format/nx.h"

#define LO32(x) ((x) & 0xffffffff)
#define HI32(x) ((x) >> 32)

static uint64_t
_nx_make_checksum(void const *data, size_t start, size_t length, uint64_t init)
{
    uint64_t        lo, hi;
    uint32_t const *words = (uint32_t const *)data;
    size_t          n, nwords = length / sizeof(words[0]);

    lo = LO32(init);
    hi = HI32(init);

    for (n = start / sizeof(words[0]); n < nwords; n++) {
        lo += nx_swap32(words[n]);
        hi += lo;
    }

    lo %= UINT32_MAX;
    hi %= UINT32_MAX;

    return (hi << 32) | lo;
}

uint64_t
nx_checksum_make(void const *data, size_t length)
{
    uint64_t csum = _nx_make_checksum(data, sizeof(csum), length, 0);
    uint64_t lo = ~((LO32(csum) + HI32(csum)) % UINT32_MAX) & UINT32_MAX;
    uint64_t hi = ~((lo + LO32(csum)) % UINT32_MAX) & UINT32_MAX;
    return (hi << 32) | lo;
}

uint64_t
nx_object_checksum(nx_object_t const *object)
{
    return nx_checksum_make(object, NX_OBJECT_SIZE);
}

bool
nx_checksum_verify(void const *data, size_t length)
{
    uint64_t csum = _nx_make_checksum(data, sizeof(csum), length, 0);
    return (_nx_make_checksum(data, 0, sizeof(csum), csum) == 0);
}

bool
nx_object_verify(nx_object_t const *object)
{
    return nx_checksum_verify(object, NX_OBJECT_SIZE);
}

bool
nx_btn_get_kvinfo(nx_btn_t const *btn, nx_btn_t const *btntop,
        uint32_t n, btn_kvinfo_t *kvi)
{
    bt_fixed_t const *bt;

    if (btn == NULL || btntop == NULL || kvi == NULL)
        return false;

    if (n >= nx_swap32(btn->btn_nkeys))
        return false;

    bt = NX_BTN_FIXED(btntop);
    if (nx_swap16(btn->btn_flags) & NX_BTN_FLAG_COMPRESSED) {
        kvi->key_offset = nx_swap16(NX_BTN_CSLOT(btn, n)->key_offset);
        kvi->key_size   = 0;
        kvi->val_offset = nx_swap16(NX_BTN_CSLOT(btn, n)->val_offset);
        kvi->val_size   = 0;

        if (kvi->key_offset != NX_BTN_NONE) {
            kvi->key_size = nx_swap32(bt->bt_key_size);
        }

        switch (kvi->val_offset) {
            case NX_BTN_NONE:
                break;
            case NX_BTN_NODE:
                kvi->val_size = NX_BTN_NODE;
                break;
            default:
                if (nx_swap16(btn->btn_flags) & NX_BTN_FLAG_LEAF) {
                    kvi->val_size = nx_swap32(bt->bt_val_size);
                } else {
                    kvi->val_size = sizeof(uint64_t);
                }
                break;
        }
    } else {
        kvi->key_offset = nx_swap16(NX_BTN_SLOT(btn, n)->key_offset);
        kvi->key_size   = nx_swap16(NX_BTN_SLOT(btn, n)->key_size);
        kvi->val_offset = nx_swap16(NX_BTN_SLOT(btn, n)->val_offset);
        kvi->val_size   = nx_swap16(NX_BTN_SLOT(btn, n)->val_size);
    }

    return true;
}

bool
nx_btn_get_kvptrs(nx_btn_t const *btn, nx_btn_t const *btntop,
        btn_kvinfo_t const *kvi, void const **keyp, void const **valp)
{
    bt_fixed_t const *bt;
    intptr_t          value_bias = 0;

    if (btn == NULL || btntop == NULL || kvi == NULL)
        return false;

    bt = NX_BTN_FIXED(btntop);
    if (nx_swap16(btn->btn_flags) & NX_BTN_FLAG_FIXED) {
        value_bias = -sizeof(bt_fixed_t);
    }

    if (keyp != NULL) {
        if (kvi->key_offset == NX_BTN_NONE) {
            *keyp = NULL;
        } else {
            *keyp = (void const *)((uintptr_t)NX_BTN_DATA(btn) +
                    nx_swap16(btn->btn_table_space.len) +
                    kvi->key_offset);
        }
    }
    if (valp != NULL) {
        if (kvi->val_offset == NX_BTN_NONE || kvi->val_offset == NX_BTN_NODE) {
            *valp = NULL;
        } else {
            *valp = (void const *)((uintptr_t)btn +
                    (nx_swap32(bt->bt_node_size) + value_bias -
                     sizeof(nx_btn_t)) - kvi->val_offset +
                    sizeof(nx_btn_t));
        }
    }

    return true;
}

bool
nx_btn_traverse(nx_btn_t const *btn, nx_btn_t const *btntop,
        nx_btn_traverse_callback_t callback, void *opaque)
{
    size_t n;

    if (btn == NULL || callback == NULL)
        return false;

    if (btntop == NULL) {
        if (NX_OBJECT_GET_TYPE(nx_swap32(NX_OBJECT(btn)->o_type)) !=
                NX_OBJECT_TYPE_BTREE_NODE)
            return false;

        btntop = btn;
    }

    for (n = 0;; n++) {
        btn_kvinfo_t  kvi;
        void const   *key;
        void const   *val;

        if (!nx_btn_get_kvinfo(btn, btntop, n, &kvi))
            break;
        if (!nx_btn_get_kvptrs(btn, btntop, &kvi, &key, &val))
            break;

        if (!(*callback)(opaque, key, kvi.key_size, val, kvi.val_size))
            break;
    }

    return true;
}
