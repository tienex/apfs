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

#ifndef __nx_format_apfs_h
#define __nx_format_apfs_h

#include "nx/format/nx.h"

#pragma pack(push, 1)

typedef struct _apfs_cryptoman {
    uint16_t cp_major;
    uint16_t cp_minor;
    uint32_t cp_flags;
} apfs_cryptoman_t;

typedef struct _apfs_modified_by {
    char     id[32];
    uint64_t timestamp;
    uint64_t last_xid;
} apfs_modified_by_t;

typedef struct _apfs_fs {
    nx_object_t        apfs_o;
    uint32_t           apfs_signature;
    uint32_t           apfs_fs_index;
    uint64_t           apfs_features;
    uint64_t           apfs_readonly_compatible_features;
    uint64_t           apfs_incompatible_features;
    uint64_t           apfs_unmount_time;
    uint64_t           apfs_fs_reserve_block_count;
    uint64_t           apfs_quota_block_count;
    uint64_t           apfs_fs_alloc_count;
    apfs_cryptoman_t   apfs_cryptoman;
    uint32_t           apfs_mt_extentref_tree_type;
    uint32_t           apfs_mt_snap_meta_tree_type;
    uint64_t           apfs_mt_omap_oid;
    uint32_t           apfs_extentref_tree_type;
    uint32_t           apfs_snap_meta_tree_type;
    uint64_t           apfs_omap_oid;
    uint64_t           apfs_root_tree_oid;
    uint64_t           apfs_extentref_tree_oid;
    uint64_t           apfs_unknown_x98;
    uint64_t           apfs_revert_to_xid;
    uint64_t           apfs_sblock_oid;
    uint64_t           apfs_unknown_xB0;
    uint64_t           apfs_unknown_xB8;
    uint64_t           apfs_unknown_xC0;
    uint64_t           apfs_unknown_xC8;
    uint64_t           apfs_unknown_xD0;
    uint64_t           apfs_unknown_xD8;
    uint64_t           apfs_total_blocks_alloced;
    uint64_t           apfs_total_blocks_freed;
    nx_uuid_t          apfs_vol_uuid;
    uint64_t           apfs_last_mod_time;
    uint64_t           apfs_fs_flags;
    apfs_modified_by_t apfs_formatted_by;
    apfs_modified_by_t apfs_modified_by[8];
    char               apfs_volname[256];
    uint32_t           apfs_next_doc_id;
    uint32_t           apfs_unknown_x3C4;
    uint64_t           apfs_unknown_x3C8;
    uint64_t           apfs_crypt_flags;
} apfs_fs_t;

#define APFS_FS_SIGNATURE 0x42535041 /* APSB */

#define APFS_FS_INCOMPATIBLE_FEATURE_CASE_SENSITIVE 1

#define APFS_OBJECT_TYPE_SNAP_METADATA  0x1
#define APFS_OBJECT_TYPE_OBJECT_EXTENT  0x2
#define APFS_OBJECT_TYPE_INODE          0x3
#define APFS_OBJECT_TYPE_XATTR          0x4
#define APFS_OBJECT_TYPE_SIBLING        0x5
#define APFS_OBJECT_TYPE_OBJECT_DSTREAM 0x6
#define APFS_OBJECT_TYPE_CRYPTO         0x7
#define APFS_OBJECT_TYPE_FILE_EXTENT    0x8
#define APFS_OBJECT_TYPE_DREC           0x9
#define APFS_OBJECT_TYPE_DIRSTATS       0xA
#define APFS_OBJECT_TYPE_SNAP_NAME      0xB
#define APFS_OBJECT_TYPE_SIBLING_MAP    0xC

#define APFS_OBJECT_ID_TYPE(OBJ_ID) ((unsigned)((OBJ_ID) >> 60))
#define APFS_OBJECT_ID_ID(OBJ_ID)   ((OBJ_ID) & (((uint64_t)1 << 60) - 1))

#define APFS_OBJECT_MAKE(TYPE, ID)  ((((uint64_t)(APFS_OBJECT_TYPE_##TYPE) & 0xf) << 60) | \
                                     ((ID) & (((uint64_t)1 << 60) - 1)))

typedef struct _apfs_xfield_entry {
    uint8_t  xf_type;
    uint8_t  xf_flags;
    uint16_t xf_len;
} apfs_xfield_entry_t;

typedef struct _apfs_xfield {
    uint16_t            xf_num_exts;
    uint16_t            xf_used_data;
    apfs_xfield_entry_t xf_entries[1];
} apfs_xfield_t;

#define APFS_DREC_EXT_SIBLING_ID        0x1
#define APFS_INO_EXT_TYPE_UNKNOWN_x2    0x2
#define APFS_INO_EXT_TYPE_DOCUMENT_ID   0x3
#define APFS_INO_EXT_TYPE_NAME          0x4
#define APFS_INO_EXT_TYPE_UNKNOWN_x5    0x5
#define APFS_INO_EXT_TYPE_UNKNOWN_x6    0x6
#define APFS_INO_EXT_TYPE_UNKNOWN_x7    0x7
#define APFS_INO_EXT_TYPE_DSTREAM       0x8
#define APFS_INO_EXT_TYPE_UNKNOWN_x9    0x9
#define APFS_INO_EXT_TYPE_DIR_STATS_KEY 0xA
#define APFS_INO_EXT_TYPE_FS_UUID       0xB
#define APFS_INO_EXT_TYPE_UNKNOWN_xC    0xC
#define APFS_INO_EXT_TYPE_SPARSE_BYTES  0xD
#define APFS_INO_EXT_TYPE_DEVICE        0xE

#define APFS_XFIELD_DATA(XFIELD) \
    ((void const *)(&(XFIELD)->xf_entries[nx_swap16((XFIELD)->xf_num_exts)]))

#define APFS_XFIELD_LENGTH_MASK 0x1fff8
#define APFS_XFIELD_ALIGN_LENGTH(LENGTH) \
    (((LENGTH) + 7) & APFS_XFIELD_LENGTH_MASK)

typedef struct _apfs_dstream {
    uint64_t  dstream_size;
    uint64_t  dstream_alloced_size;
    uint64_t  dstream_default_crypto_id;
    nx_uuid_t dstream_uuid;
} apfs_dstream_t;

#define APFS_DEVICE_SPEC_MAJOR(x) (((x) >> 24) & 0xff)
#define APFS_DEVICE_SPEC_MINOR(x) ((x) & 0xffffff)

/*
 * FS Root Node: APFS_OBJECT_TYPE_SNAP_METADATA
 */

typedef struct _apfs_snap_metadata_key {
    uint64_t obj_id; /* XID */
} apfs_snap_metadata_key_t;

typedef struct _apfs_snap_metadata_value {
    uint64_t extentref_tree_oid;
    uint64_t sblock_oid;
    uint64_t unknown_x10;
    uint64_t unknown_x18;
    uint64_t inum;
    uint32_t extentref_tree_type;
    uint32_t flags;
    uint16_t name_len;
} apfs_snap_metadata_value_t;

/*
 * apfs_snap_metadata_value_t::flags
 *    if 0x1 is set, extentref_tree_oid can be zero.
 */

/*
 * FS Root Node: APFS_OBJECT_TYPE_OBJECT_EXTENT
 */

typedef struct _apfs_object_extent_key {
    uint64_t obj_id;
} apfs_object_extent_key_t;

typedef struct _apfs_object_extent_value {
    uint64_t flags_len;
    uint64_t owning_obj_id;
    uint32_t unknown_x10;
} apfs_object_extent_value_t;

#define APFS_OBJECT_EXTENT_VALUE_FLAGS(VALUE) \
 ((uint32_t)(nx_swap64((VALUE)->flags_len) >> 56))
#define APFS_OBJECT_EXTENT_VALUE_LENGTH(VALUE) \
 (nx_swap64((VALUE)->flags_len) & ((((uint64_t)1) << 56) - 1))

/*
 * FS Root Node: APFS_OBJECT_TYPE_INODE
 */

typedef struct _apfs_inode_key {
    uint64_t obj_id;
} apfs_inode_key_t;

typedef struct _apfs_inode_value {
    uint64_t parent_id;
    uint64_t private_id;
    uint64_t creation_timestamp;
    uint64_t modification_timestamp;
    uint64_t changed_timestamp;
    uint64_t access_timestamp;
    uint64_t internal_flags;
    uint32_t nchildren;
    uint32_t default_protection_class;
    uint32_t unknown_x40;
    uint32_t bsd_flags;
    uint32_t user_id;
    uint32_t group_id;
    uint16_t mode;
    uint16_t pad1;
    uint64_t pad2;
} apfs_inode_value_t;

/*
 * apfs_inode_value_t::internal_flags
 *
 *   if 0x200 is set, sparse?
 */

#define APFS_INODE_VALUE_FLAG_SPARSE    0x0200

/*
 * apfs_inode_value_t::mode (matching Darwin)
 */

#define APFS_INODE_MODE_IFMT   0170000
#define APFS_INODE_MODE_IFIFO  0010000
#define APFS_INODE_MODE_IFCHR  0020000
#define APFS_INODE_MODE_IFDIR  0040000
#define APFS_INODE_MODE_IFBLK  0060000
#define APFS_INODE_MODE_IFREG  0100000
#define APFS_INODE_MODE_IFLNK  0120000
#define APFS_INODE_MODE_IFSOCK 0140000
#define APFS_INODE_MODE_IFWHT  0160000

/*
 * apfs_inode_value_t::bsd_flags (matching Darwin)
 */

#define APFS_INODE_BSD_FLAGS_USER_SETTABLE    0x0000ffff
#define APFS_INODE_BSD_FLAGS_USER_NODUMP      0x00000001
#define APFS_INODE_BSD_FLAGS_USER_IMMUTABLE   0x00000002
#define APFS_INODE_BSD_FLAGS_USER_APPEND      0x00000004
#define APFS_INODE_BSD_FLAGS_USER_OPAQUE      0x00000008
#define APFS_INODE_BSD_FLAGS_USER_COMPRESSED  0x00000020
#define APFS_INODE_BSD_FLAGS_USER_TRACKED     0x00000040
#define APFS_INODE_BSD_FLAGS_USER_DATAVAULT   0x00000080
#define APFS_INODE_BSD_FLAGS_USER_HIDDEN      0x00008000

#define APFS_INODE_BSD_FLAGS_SUPER_SUPPORTED  0x001f0000
#define APFS_INODE_BSD_FLAGS_SUPER_SETTABLE   0xffff0000
#define APFS_INODE_BSD_FLAGS_SUPER_ARCHIVED   0x00010000
#define APFS_INODE_BSD_FLAGS_SUPER_IMMUTABLE  0x00020000
#define APFS_INODE_BSD_FLAGS_SUPER_APPEND     0x00040000
#define APFS_INODE_BSD_FLAGS_SUPER_RESTRICTED 0x00080000
#define APFS_INODE_BSD_FLAGS_SUPER_NOUNLINK   0x00100000

/*
 * FS Root Node: APFS_OBJECT_TYPE_XATTR
 */

typedef struct _apfs_xattr_key {
    uint64_t obj_id;
    uint16_t name_len;
    char     name[1];
} apfs_xattr_key_t;

typedef struct _apfs_xattr_value {
    uint16_t flags;
    uint16_t xdata_len;
} apfs_xattr_value_t;

typedef struct _apfs_xattr_indirect_value {
    apfs_xattr_value_t base;
    uint64_t           xattr_obj_id;
    apfs_dstream_t     dstream;
} apfs_xattr_indirect_value_t;

/* special xattrs */
#define APFS_XATTR_NAME_FINDERINFO   "com.apple.FinderInfo"
#define APFS_XATTR_NAME_RESOURCEFORK "com.apple.ResourceFork"
#define APFS_XATTR_NAME_SECURITY     "com.apple.system.Security"
#define APFS_XATTR_NAME_SYMLINK      "com.apple.fs.symlink"

/* apfs_xattr_value_t::flags
 *
 *   valid mask = 0x7
 *   if 0x1|0x2 is set, is invalid
 *   if 0x4 is set, then name must be 'com.apple.fs.symlink'
 *   if 0x1 is set, then xdata_len must be 8 bytes for xattr_obj_id and
 *                  apfs_dstream_t (total 48 bytes)
 *   if 0x2 is set, then xdata_len must be less than 3805 bytes (?)
 */

#define APFS_XATTR_VALUE_FLAG_INDIRECT 0x1
#define APFS_XATTR_VALUE_FLAG_INLINE   0x2
#define APFS_XATTR_VALUE_FLAG_SYMLINK  0x4

/*
 * FS Root Node: APFS_OBJECT_TYPE_SIBLING
 */

typedef struct _apfs_sibling_key {
    uint64_t obj_id;
    uint64_t sibling_id;
} apfs_sibling_key_t;

typedef struct _apfs_sibling_value {
    uint64_t parent_id;
    uint16_t name_len;
} apfs_sibling_value_t;

/*
 * FS Root Node: APFS_OBJECT_TYPE_OBJECT_DSTREAM
 */

typedef struct _apfs_object_dstream_key {
    uint64_t obj_id;
} apfs_object_dstream_key_t;

typedef struct _apfs_object_dstream_value {
    uint32_t count;
} apfs_object_dstream_value_t;

/*
 * FS Root Node: APFS_OBJECT_TYPE_CRYPTO
 */

typedef struct _apfs_crypto_key {
    uint64_t obj_id;
} apfs_crypto_key_t;

typedef struct _apfs_crypto_state {
    uint16_t major_version;
    uint16_t minor_version;
    uint32_t cpflags;
    uint32_t persistent_class;
    uint32_t key_os_version;
    uint16_t key_revision;
    uint16_t key_len;
} apfs_crypto_state_t;

typedef struct _apfs_crypto_value {
    uint32_t            refcnt;
    apfs_crypto_state_t state;
} apfs_crypto_value_t;

/*
 * FS Root Node: APFS_OBJECT_TYPE_FILE_EXTENT
 */

typedef struct _apfs_file_extent_key {
    uint64_t obj_id;
    uint64_t offset;
} apfs_file_extent_key_t;

typedef struct _apfs_file_extent_value {
    uint64_t flags_len;
    uint64_t phys_block_num;
    uint64_t crypto_id;
} apfs_file_extent_value_t;

#define APFS_FILE_EXTENT_VALUE_FLAGS(VALUE) \
 ((uint32_t)(nx_swap64((VALUE)->flags_len) >> 56))
#define APFS_FILE_EXTENT_VALUE_LENGTH(VALUE) \
 (nx_swap64((VALUE)->flags_len) & ((((uint64_t)1) << 56) - 1))

/*
 * apfs_file_extent_value_t::flags
 *
 *      valid mask = 0x1
 *      0x1 is not set if crypto_id is valid?
 */

#define APFS_FILE_EXTENT_VALUE_FLAG_NOCRYPTO 0x1

/*
 * FS Root Node: APFS_OBJECT_TYPE_DREC
 */

typedef struct _apfs_drec_key {
    uint64_t obj_id;
    union {
        struct {
            uint16_t name_len;
            char     name[1];
        } nohash;
        struct {
            uint32_t name_len_hash;
            char     name[1];
        } hashed;
    };
} apfs_drec_key_t;

#define APFS_DREC_HASHED_NAME_LENGTH(KEY) \
    (nx_swap32((KEY)->hashed.name_len_hash) & 0x3ff)
#define APFS_DREC_HASHED_NAME_HASH(KEY) \
    (nx_swap32((KEY)->hashed.name_len_hash) >> 10)
#define APFS_DREC_NOHASH_NAME_LENGTH(KEY) \
    (nx_swap16((KEY)->nohash.name_len))

#define APFS_DREC_DEAD_OBJ_ID         3
#define APFS_DREC_DEAD_SUFFIX         "-dead"

typedef struct _apfs_drec_value {
    uint64_t file_id;
    uint64_t timestamp;
    uint16_t flags_item_type;
} apfs_drec_value_t;

#define APFS_DREC_VALUE_FLAGS(VALUE) \
    (nx_swap16((VALUE)->flags_item_type) >> 4)
#define APFS_DREC_VALUE_ITEM_TYPE(VALUE) \
    (nx_swap16((VALUE)->flags_item_type) & 0xf)

#define APFS_DREC_ROOT_FILE_ID        2
#define APFS_DREC_ROOT_NAME           "root"
#define APFS_DREC_PRIVATE_DIR_FILE_ID 3
#define APFS_DREC_PRIVATE_DIR_NAME    "private-dir"

#define APFS_ITEM_TYPE_FIFO          1
#define APFS_ITEM_TYPE_CHAR_DEVICE   2
#define APFS_ITEM_TYPE_DIRECTORY     4
#define APFS_ITEM_TYPE_BLOCK_DEVICE  6
#define APFS_ITEM_TYPE_REGULAR       8
#define APFS_ITEM_TYPE_SYMBOLIC_LINK 10
#define APFS_ITEM_TYPE_SOCKET        12
#define APFS_ITEM_TYPE_WHITEOUT      14

/*
 * apfs_drec_value_t::flags
 *
 *   valid mask = 0xfff?
 *   0x20 if sparse?
 */

#define APFS_DREC_VALUE_FLAG_SPARSE 0x20

/*
 * FS Root Node: APFS_OBJECT_TYPE_DIRSTATS
 */

typedef struct _apfs_dirstats_key {
    uint64_t obj_id;
} apfs_dirstats_key_t;

typedef struct _apfs_dirstats_value {
    uint64_t unknown_x0;
    uint64_t unknown_x8;
    uint64_t chained_key;
    uint64_t gen_count;
} apfs_dirstats_value_t;

/*
 * FS Root Node: APFS_OBJECT_TYPE_SNAP_NAME
 */

typedef struct _apfs_snap_name_key {
    uint64_t obj_id;
    uint16_t name_len;
} apfs_snap_name_key_t;

typedef struct _apfs_snap_name_value {
    uint64_t snap_xid;
} apfs_snap_name_value_t;

/*
 * FS Root Node: APFS_OBJECT_TYPE_SIBLING_MAP
 */

typedef struct _apfs_sibling_map_key {
    uint64_t obj_id;
} apfs_sibling_map_key_t;

typedef struct _apfs_sibling_map_value {
    uint64_t file_id;
} apfs_sibling_map_value_t;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

char *apfs_format_time(uint64_t timestamp, char *buf, size_t bufsiz, bool iso);
uint32_t apfs_hash_name(char const *name, size_t namelen, bool insensitive);

void apfs_fs_dump(nx_dumper_t *dumper, apfs_fs_t const *fs);

void apfs_object_dump(nx_dumper_t *dumper,
        void const *key, size_t key_len,
        void const *value, size_t value_len);
void apfs_snap_metadata_dump(nx_dumper_t *dumper,
        apfs_snap_metadata_key_t const *key,
        apfs_snap_metadata_value_t const *value);
void apfs_object_extent_dump(nx_dumper_t *dumper,
        apfs_object_extent_key_t const *key,
        apfs_object_extent_value_t const *value);
void apfs_inode_dump(nx_dumper_t *dumper,
        apfs_inode_key_t const *key,
        apfs_inode_value_t const *value,
        bool xfields);
void apfs_xattr_dump(nx_dumper_t *dumper,
        apfs_xattr_key_t const *key,
        apfs_xattr_value_t const *value);
void apfs_sibling_dump(nx_dumper_t *dumper,
        apfs_sibling_key_t const *key,
        apfs_sibling_value_t const *value);
void apfs_object_dstream_dump(nx_dumper_t *dumper,
        apfs_object_dstream_key_t const *key,
        apfs_object_dstream_value_t const *value);
void apfs_crypto_dump(nx_dumper_t *dumper,
        apfs_crypto_key_t const *key,
        apfs_crypto_value_t const *value);
void apfs_file_extent_dump(nx_dumper_t *dumper,
        apfs_file_extent_key_t const *key,
        apfs_file_extent_value_t const *value);
void apfs_drec_dump(nx_dumper_t *dumper,
        apfs_drec_key_t const *key,
        apfs_drec_value_t const *value,
        bool xfields);
void apfs_dirstats_dump(nx_dumper_t *dumper,
        apfs_dirstats_key_t const *key,
        apfs_dirstats_value_t const *value);
void apfs_snap_name_dump(nx_dumper_t *dumper,
        apfs_snap_name_key_t const *key,
        apfs_snap_name_value_t const *value);
void apfs_sibling_map_dump(nx_dumper_t *dumper,
        apfs_sibling_map_key_t const *key,
        apfs_sibling_map_value_t const *value);

#ifdef __cplusplus
}
#endif

#endif  /* !__nx_format_apfs_h */
