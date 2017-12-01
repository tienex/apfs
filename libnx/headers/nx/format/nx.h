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

#ifndef __nx_format_nx_h
#define __nx_format_nx_h

#include "nx/format/base.h"
#include "nx/format/swap.h"
#include "nx/format/nx_dumper.h"

#pragma pack(push, 1)

#define NX_OBJECT_SIZE 4096

typedef struct _nx_object {
    uint64_t o_checksum;
    uint64_t o_oid;
    uint64_t o_xid;
    uint32_t o_type;
    uint32_t o_subtype;
} nx_object_t;

#define NX_OBJECT_TYPE_CONTAINER      0x01
#define NX_OBJECT_TYPE_BTREE_ROOT     0x02
#define NX_OBJECT_TYPE_BTREE_NODE     0x03
#define NX_OBJECT_TYPE_MIDDLE_TREE    0x04
#define NX_OBJECT_TYPE_SPACEMAN       0x05
#define NX_OBJECT_TYPE_CAB            0x06 /* Chunk Allocation Block? */
#define NX_OBJECT_TYPE_CIB            0x07 /* Chunk Info Block? */
#define NX_OBJECT_TYPE_SPACEMAN_IP    0x08 /* Space Manager Internal Pool */
#define NX_OBJECT_TYPE_UNKNOWN9       0x09
#define NX_OBJECT_TYPE_UNKNOWNA       0x0a
#define NX_OBJECT_TYPE_OBJECT_MAP     0x0b
#define NX_OBJECT_TYPE_CHECKPOINT_MAP 0x0c
#define NX_OBJECT_TYPE_APFS_VOLUME    0x0d
#define NX_OBJECT_TYPE_UNKNOWNE       0x0e
#define NX_OBJECT_TYPE_UNKNOWNF       0x0f
#define NX_OBJECT_TYPE_UNKNOWN10      0x10
#define NX_OBJECT_TYPE_NEXT_REAP      0x11 /* Next Reap */
#define NX_OBJECT_TYPE_NEXT_REAP_LIST 0x12 /* Next Reap List */
#define NX_OBJECT_TYPE_UNKNOWN13      0x13 /* ??? */
#define NX_OBJECT_TYPE_UNKNOWN14      0x14 /* ??? */
#define NX_OBJECT_TYPE_UNKNOWN15      0x15 /* ??? */
#define NX_OBJECT_TYPE_WBC            0x16 /* Write Back Chunk? */
#define NX_OBJECT_TYPE_WBC_LIST       0x17 /* Write Back Chunk? List */

#define NX_OBJECT_FLAG_OBJECT         0x00000000 /* oid is in object map */
#define NX_OBJECT_FLAG_DIRECT         0x40000000 /* oid is a direct block */
#define NX_OBJECT_FLAG_CHECKPOINT     0x80000000 /* oid is in checkpoint map */

#define NX_OBJECT_CPMAP_TYPE(TYPE) \
    ((uint32_t)(NX_OBJECT_FLAG_CHECKPOINT | ((NX_OBJECT_TYPE_##TYPE) & 0x3fffffff)))
#define NX_OBJECT_DIRECT_TYPE(TYPE) \
    ((uint32_t)(NX_OBJECT_FLAG_DIRECT | ((NX_OBJECT_TYPE_##TYPE) & 0x3fffffff)))
#define NX_OBJECT_OMAP_TYPE(TYPE) \
    ((uint32_t)(NX_OBJECT_FLAG_OBJECT | ((NX_OBJECT_TYPE_##TYPE) & 0x3fffffff)))

#define NX_OBJECT_GET_TYPE(TYPE) ((TYPE) & 0x3fffffff)

typedef struct _nx_uuid {
    uint8_t bytes[16];
} nx_uuid_t;

typedef struct _nx_super {
    nx_object_t nx_o;
    uint32_t    nx_signature;
    uint32_t    nx_block_size;
    uint64_t    nx_block_count;
    uint64_t    nx_features;
    uint64_t    nx_readonly_compatible_features;
    uint64_t    nx_incompatible_features;
    nx_uuid_t   nx_uuid;
    uint64_t    nx_next_oid;
    uint64_t    nx_next_xid;
    uint32_t    nx_xp_desc_blocks; /* The number of blocks containing descriptors */
    uint32_t    nx_xp_data_blocks;
    uint64_t    nx_xp_desc_first;  /* The first block containing descriptors */
    uint64_t    nx_xp_data_first;
    uint32_t    nx_xp_desc_next;   /* The next index into the descriptors block */
    uint32_t    nx_xp_data_next;
    uint32_t    nx_xp_desc_index;  /* The index into the descriptors block for this entry */
    uint32_t    nx_xp_desc_len;    /* The number of descriptors indexes, plus 1 */
    uint32_t    nx_xp_data_index;
    uint32_t    nx_xp_data_len;
    uint64_t    nx_spaceman_oid;
    uint64_t    nx_omap_oid;
    uint64_t    nx_reaper_oid;
    uint32_t    nx_padding0;
    uint32_t    nx_max_file_systems;
    uint64_t    nx_fs_oid[100];
    uint64_t    nx_padding1[32];
    uint64_t    nx_blocked_out;
    uint64_t    nx_blocked_out_len;
    uint64_t    nx_padding2;
    uint64_t    nx_flags;
    uint64_t    nx_padding3[3];
    uint64_t    nx_keybag_data;
    uint64_t    nx_keybag_data_len;
    uint64_t    nx_ephemeral_info[4];
    uint64_t    nx_padding4;
    uint64_t    nx_fusion_mt_oid;
    uint64_t    nx_fusion_wbc_oid;
    uint64_t    nx_fusion_wbc;
    uint64_t    nx_fusion_wbc_len;
} nx_super_t;

#define NX_SUPER_SIGNATURE 0x4253584e /* NXSB */

typedef struct _nx_efi_jumpstart {
    nx_object_t ejs_o;
    uint32_t    ejs_signature;
    uint32_t    ejs_version;
    uint32_t    ejs_file_length;
    uint32_t    ejs_extent_count;
} nx_efi_jumpstart_t;

#define NX_EFI_JUMPSTART_SIGNATURE 0x5244534a /* JSDR */
#define NX_EFI_JUMPSTART_VERSION_1 1

typedef struct _nx_cib_entry {
    uint64_t cib_xid;
    uint64_t cib_paddr;
    uint32_t cib_size;
    uint32_t cib_unknown_x10;
    uint64_t cib_chunk;
} nx_cib_entry_t;

typedef struct _nx_cib {
    nx_object_t    cib_o;
    uint32_t       cib_flags;
    uint32_t       cib_count;
    nx_cib_entry_t cib_map[1];
} nx_cib_t;

typedef struct _nx_cpm_entry {
    uint32_t cpm_type;
    uint32_t cpm_subtype;
    uint64_t cpm_size;
    uint64_t cpm_fs_oid;
    uint64_t cpm_oid;
    uint64_t cpm_paddr;
} nx_cpm_entry_t;

typedef struct _nx_cpm {
    nx_object_t    cpm_o;
    uint32_t       cpm_flags;
    uint32_t       cpm_count;
    nx_cpm_entry_t cpm_map[1];
} nx_cpm_t;

typedef struct _nx_spaceman {
    nx_object_t    sm_o;
    uint32_t       sm_block_size;
    uint32_t       sm_blocks_per_chunk;
    uint32_t       sm_chunks_per_cib;
    uint32_t       sm_cibs_per_cab;
    uint64_t       sm_block_count;
    uint64_t       sm_chunk_count;
    uint32_t       sm_cib_count;
    uint32_t       sm_cab_count;
    uint64_t       sm_free_count;
    uint64_t       sm_unknown_x50;
    uint64_t       sm_unknown_x58;
    uint64_t       sm_tier2_block_count;
    uint64_t       sm_tier2_chunk_count;
    uint32_t       sm_tier2_cib_count;
    uint32_t       sm_tier2_cab_count;
    uint64_t       sm_tier2_free_count;
    uint64_t       sm_unknown_x80;
    uint64_t       sm_unknown_x88;
    uint64_t       sm_unknown_x90;
    uint64_t       sm_ip_block_count;
    uint32_t       sm_ip_bm_block_count;
    uint32_t       sm_ip_bitmap_block_count;
    uint64_t       sm_ip_bitmap_block_first;
    uint64_t       sm_ip_block_first;
    uint64_t       sm_unknown_xB8;
    uint64_t       sm_ip_free_queue_count;
    uint64_t       sm_unknown_xC8;
    uint64_t       sm_unknown_xD0;
    uint64_t       sm_unknown_xD8;
    uint64_t       sm_unknown_xE0;
    uint64_t       sm_unknown_xE8;
    uint64_t       sm_free_queue_count;
    uint64_t       sm_tree_oid;
    uint64_t       sm_unknown_x100;
    uint64_t       sm_unknown_x108;
    uint64_t       sm_unknown_x110;
    uint64_t       sm_tier2_free_queue_count;
    uint64_t       sm_unknown_x120;
    uint64_t       sm_unknown_x128;
    uint64_t       sm_unknown_x130;
    uint64_t       sm_unknown_x138;
    uint16_t       sm_ip_bm_next_array_free;
    uint16_t       sm_unknown_x142;
    uint32_t       sm_xid_offset;
    uint32_t       sm_bm_offset;
    uint32_t       sm_unknown_x14C;
} nx_spaceman_t;

typedef struct _nx_omap {
    nx_object_t om_o;
    uint32_t    om_flags;
    uint32_t    om_snap_count;
    uint32_t    om_tree_type;
    uint32_t    om_snapshot_tree_type;
    uint64_t    om_tree_oid;
    uint64_t    om_snapshot_tree_oid;
    uint64_t    om_most_recent_snap;
    uint64_t    om_pending_revert_min;
    uint64_t    om_pending_revert_max;
} nx_omap_t;

#define NX_OMAP_FLAG_SNAPSHOT (1 << 0)

typedef struct _nx_omap_key {
    uint64_t ok_oid;
    uint64_t ok_xid;
} nx_omap_key_t;

typedef union _nx_omap_value {
    uint64_t ov_paddr;
    struct {
        uint64_t ov_flags;
        uint64_t ov_oid;
    };
} nx_omap_value_t;

typedef struct _bt_fixed {
    uint32_t bt_flags;
    uint32_t bt_node_size;
    uint32_t bt_key_size;
    uint32_t bt_val_size;
    uint32_t bt_longest_key;
    uint32_t bt_longest_val;
    uint64_t bt_key_count;
    uint64_t bt_node_count;
} bt_fixed_t;

typedef struct _btn_space {
    uint16_t offset;
    uint16_t len;
} btn_space_t;

/* Compressed slot */
typedef struct _btn_cslot {
    uint16_t key_offset;
    uint16_t val_offset;
} btn_cslot_t;

/* Uncompressed slot */
typedef struct _btn_slot {
    uint16_t key_offset;
    uint16_t key_size;
    uint16_t val_offset;
    uint16_t val_size;
} btn_slot_t;

typedef struct _nx_btn {
    nx_object_t btn_o;
    uint16_t    btn_flags;
    uint16_t    btn_level;
    uint32_t    btn_nkeys;
    btn_space_t btn_table_space;
    btn_space_t btn_free_space;
    btn_space_t btn_key_free_list;
    btn_space_t btn_val_free_list;
} nx_btn_t;

#define NX_BTN_DATA(BTN)     ((void *)((BTN) + 1))
#define NX_BTN_SLOT(BTN, N)  (&((btn_slot_t *)NX_BTN_DATA(BTN))[N])
#define NX_BTN_CSLOT(BTN, N) (&((btn_cslot_t *)NX_BTN_DATA(BTN))[N])

#define NX_BTN_MAX_KEY_SIZE 832
#define NX_BTN_MAX_VAL_SIZE 3808

#define NX_BTN_FLAG_FIXED      (1 << 0) /* Subtract the size of fixed part */
#define NX_BTN_FLAG_LEAF       (1 << 1) /* This node is a leaf, the value size
                                         * is taken from the fixed part, else
                                         * this is a non-leaf node and the
                                         * value size defaults to 8 and it's
                                         * an OID pointing to the rest of the
                                         * tree.
                                         */
#define NX_BTN_FLAG_COMPRESSED (1 << 2) /* Key/Value are packed */

#define NX_BTN_NONE 0xffff
#define NX_BTN_NODE 0xfffe /* Intermediate Node? */

typedef struct _btn_kvinfo {
    uint16_t key_offset;
    uint16_t key_size;
    uint16_t val_offset;
    uint16_t val_size;
} btn_kvinfo_t;

#define NX_BTN_FIXED(BTREE) NX_BTN_FIXED_V(BTREE, NX_OBJECT_SIZE)
#define NX_BTN_FIXED_V(BTREE, BSIZE) \
    ((bt_fixed_t *)((uintptr_t)(BTREE) + BSIZE - sizeof(bt_fixed_t)))

#pragma pack(pop)

typedef bool (*nx_btn_traverse_callback_t)(void *opaque,
        void const *key, size_t key_size,
        void const *val, size_t val_size);

#define NX_OBJECT(x) ((nx_object_t const *)(x))

#ifdef __cplusplus
extern "C" {
#endif

char const *nx_object_type_name_r(uint16_t type, uint32_t subtype,
        char *buf, size_t bufsiz);
char const *nx_object_type_name(uint16_t type, uint32_t subtype);
char const *nx_object_name_r(nx_object_t const *object,
        char *buf, size_t bufsiz);
char const *nx_object_name(nx_object_t const *object);

char const *nx_uuid_format(nx_uuid_t const *uuid, char *buf, size_t bufsiz);

uint64_t nx_checksum_make(void const *data, size_t length);
uint64_t nx_object_checksum(nx_object_t const *object);

bool nx_checksum_verify(void const *data, size_t length);
bool nx_object_verify(nx_object_t const *object);

bool nx_btn_get_kvinfo(nx_btn_t const *btn, nx_btn_t const *btntop,
        uint32_t n, btn_kvinfo_t *kvi);
bool nx_btn_get_kvptrs(nx_btn_t const *btn, nx_btn_t const *btntop,
        btn_kvinfo_t const *kvi, void const **keyp, void const **valp);

bool nx_btn_traverse(nx_btn_t const *btn, nx_btn_t const *btntop,
        nx_btn_traverse_callback_t callback, void *opaque);

void nx_object_dump(nx_dumper_t *dumper, nx_object_t const *object);
void nx_super_dump(nx_dumper_t *dumper, nx_super_t const *sb);
void nx_efi_jumpstart_dump(nx_dumper_t *dumper, nx_efi_jumpstart_t const *ejs);
void nx_cib_dump(nx_dumper_t *dumper, nx_cib_t const *cib);
void nx_cpm_dump(nx_dumper_t *dumper, nx_cpm_t const *cpm);
void nx_spaceman_dump(nx_dumper_t *dumper, nx_spaceman_t const *sm);
void nx_omap_dump(nx_dumper_t *dumper, nx_omap_t const *om);
void nx_btn_dump(nx_dumper_t *dumper, nx_btn_t const *btn,
        nx_btn_t const *btntop);

#ifdef __cplusplus
}
#endif

#endif  /* !__nx_h */
