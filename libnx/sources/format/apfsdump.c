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

#include "nx/format/apfs.h"

#include "nxcompat/nxcompat.h"

#include <stdlib.h>

#include "formatting.h"

#define _HTIME(time) apfs_format_time(time, timbuf1, sizeof(timbuf1), false)
#define _ITIME(time) apfs_format_time(time, timbuf2, sizeof(timbuf2), true)

static inline bool
apfs_modified_by_is_valid(apfs_modified_by_t const *mby)
{
    return (mby->id[0] != '\0' || mby->timestamp != 0 || mby->last_xid != 0);
}

void
apfs_fs_dump(nx_dumper_t *dumper, apfs_fs_t const *fs)
{
    size_t n, indent = 0;
    char   buf[128];
    char   timbuf1[64];
    char   timbuf2[64];

    if (dumper == NULL || fs == NULL)
        return;

    nx_dumper_emit(dumper, indent, _TITLE("APFS Super") "\n");

    _nx_object_dump0(dumper, NX_OBJECT(fs));
    nx_dumper_open_container(dumper, "apfs-super");
    {
        _INC_INDENT();

        _nx_object_dump1(dumper, indent, NX_OBJECT(fs));
        nx_dumper_emit(dumper, indent,
                _LABEL("Signature")
                "{:signature/%4.4s}\n",
                (char const *)&fs->apfs_signature);
        nx_dumper_emit(dumper, indent,
                _LABEL("File System Index")
                "{:fs-index/%u}\n",
                nx_swap32(fs->apfs_fs_index));
        nx_dumper_emit(dumper, indent,
                _LABEL("Features")
                "{dw:/%#" PRIx64 "}"
                "{e:features/%" PRIu64 "}{d:/%s}\n",
                nx_swap64(fs->apfs_features),
                nx_swap64(fs->apfs_features),
                _bitflag_names(nx_swap64(fs->apfs_features), NULL, NULL,
                    buf, sizeof(buf)));
        nx_dumper_emit(dumper, indent,
                _LABEL("Read Only Compatible Features")
                "{dw:/%#" PRIx64 "}"
                "{e:ro-compatible-features/%#" PRIu64 "}{d:/%s}\n",
                nx_swap64(fs->apfs_readonly_compatible_features),
                nx_swap64(fs->apfs_readonly_compatible_features),
                _bitflag_names(nx_swap64(fs->apfs_readonly_compatible_features),
                    NULL, NULL, buf, sizeof(buf)));
        nx_dumper_emit(dumper, indent,
                _LABEL("Incompatible Features")
                "{dw:/%#" PRIx64 "}"
                "{e:incompatible-features/%" PRIu64 "}{d:/%s}\n",
                nx_swap64(fs->apfs_incompatible_features),
                nx_swap64(fs->apfs_incompatible_features),
                _bitflag_names(nx_swap64(fs->apfs_incompatible_features),
                    NULL, "\001CASE-SENSITIVE", buf, sizeof(buf)));
        nx_dumper_emit(dumper, indent,
                _LABEL("Unmount Time")
                "{d:/%s}{e:unmount-time/%s}\n",
                _HTIME(nx_swap64(fs->apfs_unmount_time)),
                _ITIME(nx_swap64(fs->apfs_unmount_time)));
        nx_dumper_emit(dumper, indent,
                _LABEL("Reserve Block Count")
                "{w:fs-reserve-block-count/%" PRIu64 "}"
                "{d:/(}"
                "{h,hn-1000,hn-decimal:fs-reserve-bytes-capacity/%" PRIu64 "}"
                "{d:/)}"
                "\n",
                nx_swap64(fs->apfs_fs_reserve_block_count),
                nx_swap64(fs->apfs_fs_reserve_block_count) * NX_OBJECT_SIZE);
        nx_dumper_emit(dumper, indent,
                _LABEL("Quota Block Count")
                "{w:quota-block-count/%" PRIu64 "}"
                "{d:/(}"
                "{h,hn-1000,hn-decimal:quota-bytes-capacity/%" PRIu64 "}"
                "{d:/)}"
                "\n",
                nx_swap64(fs->apfs_quota_block_count),
                nx_swap64(fs->apfs_quota_block_count) * NX_OBJECT_SIZE);
        nx_dumper_emit(dumper, indent,
                _LABEL("Allocated Block Count")
                "{w:fs-alloc-block-count/%" PRIu64 "}"
                "{d:/(}"
                "{h,hn-1000,hn-decimal:fs-alloc-bytes-capacity/%" PRIu64 "}"
                "{d:/)}"
                "\n",
                nx_swap64(fs->apfs_fs_alloc_count),
                nx_swap64(fs->apfs_fs_alloc_count) * NX_OBJECT_SIZE);

        nx_dumper_emit(dumper, indent, _TITLE("Crypto Manager") "\n");
        nx_dumper_open_container(dumper, "crypto-man");
        {
            _INC_INDENT();

            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Version")
                    "{d:/%u.%u}{e:version-major/%u}{e:version-minor/%u}\n",
                    nx_swap16(fs->apfs_cryptoman.cp_major),
                    nx_swap16(fs->apfs_cryptoman.cp_minor),
                    nx_swap16(fs->apfs_cryptoman.cp_major),
                    nx_swap16(fs->apfs_cryptoman.cp_minor));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Flags")
                    "{dw:/%#x}{e:flags/%u}{d:/%s}\n",
                    nx_swap32(fs->apfs_cryptoman.cp_flags),
                    nx_swap32(fs->apfs_cryptoman.cp_flags),
                    _bitflag_names(nx_swap32(fs->apfs_cryptoman.cp_flags),
                        NULL, NULL, buf, sizeof(buf)));

            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        nx_dumper_emit(dumper, indent,
                _LABEL("MT Extent Ref Tree Type?")
                "{d:/%#x}{e:mt-extentref-tree-type/%u}\n",
                nx_swap32(fs->apfs_mt_extentref_tree_type),
                nx_swap32(fs->apfs_mt_extentref_tree_type));
        nx_dumper_emit(dumper, indent,
                _LABEL("MT Snapshot Metadata Tree Type?")
                "{d:/%#x}{e:mt-snap-meta-tree-type/%u}\n",
                nx_swap32(fs->apfs_mt_snap_meta_tree_type),
                nx_swap32(fs->apfs_mt_snap_meta_tree_type));
        nx_dumper_emit(dumper, indent,
                _LABEL("MT Object Map Block #?")
                "{w:mt-omap-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_mt_omap_oid),
                nx_swap64(fs->apfs_mt_omap_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Extent Ref Tree Type")
                "{d:/%#x}{e:extentref-tree-type/%u}\n",
                nx_swap32(fs->apfs_extentref_tree_type),
                nx_swap32(fs->apfs_extentref_tree_type));
        nx_dumper_emit(dumper, indent,
                _LABEL("Snapshot Metadata Tree Type")
                "{d:/%#x}{e:extentref-tree-type/%u}\n",
                nx_swap32(fs->apfs_snap_meta_tree_type),
                nx_swap32(fs->apfs_snap_meta_tree_type));
        nx_dumper_emit(dumper, indent,
                _LABEL("Object Map Block #")
                "{w:omap-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_omap_oid),
                nx_swap64(fs->apfs_omap_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Root Tree Object ID")
                "{w:root-tree-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_root_tree_oid),
                nx_swap64(fs->apfs_root_tree_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Extent Ref Tree Block #")
                "{w:extentref-tree-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_extentref_tree_oid),
                nx_swap64(fs->apfs_extentref_tree_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Unknown Field 0x98")
                "{w:unknown-x98/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_unknown_x98),
                nx_swap64(fs->apfs_unknown_x98));
        nx_dumper_emit(dumper, indent,
                _LABEL("Revert to Checkpoint ID")
                "{w:revert-to-xid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_revert_to_xid),
                nx_swap64(fs->apfs_revert_to_xid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Snapshot Block Object ID")
                "{w:sblock-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_sblock_oid),
                nx_swap64(fs->apfs_sblock_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Unknown Field 0xB0")
                "{w:unknown-xB0/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_unknown_xB0),
                nx_swap64(fs->apfs_unknown_xB0));
        nx_dumper_emit(dumper, indent,
                _LABEL("Unknown Field 0xB8")
                "{w:unknown-xB8/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_unknown_xB8),
                nx_swap64(fs->apfs_unknown_xB8));
        nx_dumper_emit(dumper, indent,
                _LABEL("Unknown Field 0xC0")
                "{w:unknown-xC0/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_unknown_xC0),
                nx_swap64(fs->apfs_unknown_xC0));
        nx_dumper_emit(dumper, indent,
                _LABEL("Unknown Field 0xC8")
                "{w:unknown-xC8/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_unknown_xC8),
                nx_swap64(fs->apfs_unknown_xC8));
        nx_dumper_emit(dumper, indent,
                _LABEL("Unknown Field 0xD0")
                "{w:unknown-xD0/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_unknown_xD0),
                nx_swap64(fs->apfs_unknown_xD0));
        nx_dumper_emit(dumper, indent,
                _LABEL("Unknown Field 0xD8")
                "{w:unknown-xD8/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_unknown_xD8),
                nx_swap64(fs->apfs_unknown_xD8));
        nx_dumper_emit(dumper, indent,
                _LABEL("Total Blocks Allocated")
                "{w:total-blocks-alloced/%" PRIu64 "}"
                "{d:/(}"
                "{h,hn-1000,hn-decimal:total-alloced-bytes-capacity/%" PRIu64 "}"
                "{d:/)}"
                "\n",
                nx_swap64(fs->apfs_total_blocks_alloced),
                nx_swap64(fs->apfs_total_blocks_alloced) * NX_OBJECT_SIZE);
        nx_dumper_emit(dumper, indent,
                _LABEL("Total Blocks Freed")
                "{w:total-blocks-freed/%" PRIu64 "}"
                "{d:/(}"
                "{h,hn-1000,hn-decimal:total-freed-bytes-capacity/%" PRIu64 "}"
                "{d:/)}"
                "\n",
                nx_swap64(fs->apfs_total_blocks_freed),
                nx_swap64(fs->apfs_total_blocks_freed) * NX_OBJECT_SIZE);
        nx_dumper_emit(dumper, indent,
                _LABEL("Volume UUID")
                "{:vol-uuid}\n",
                nx_uuid_format(&fs->apfs_vol_uuid, buf, sizeof(buf)));
        nx_dumper_emit(dumper, indent,
                _LABEL("Last Modified Time")
                "{d:/%s}{e:last-mod-time/%s}\n",
                _HTIME(nx_swap64(fs->apfs_last_mod_time)),
                _ITIME(nx_swap64(fs->apfs_last_mod_time)));
        nx_dumper_emit(dumper, indent,
                _LABEL("Flags")
                "{dw:/%#" PRIx64 "}"
                "{e:fs-flags/%" PRIu64 "}{d:/%s}\n",
                nx_swap64(fs->apfs_fs_flags),
                nx_swap64(fs->apfs_fs_flags),
                _bitflag_names(nx_swap64(fs->apfs_fs_flags), NULL, NULL,
                    buf, sizeof(buf)));

        nx_dumper_emit(dumper, indent, _TITLE("Autorship") "\n");

        if (nx_dumper_is_text(dumper)) {
            nx_dumper_table_t *table;

            _INC_INDENT();

            table = nx_dumper_table_new(MAX_TABLE_WIDTH - indent,
                    "Role",               "%s",       NX_DUMPER_TABLE_LEFT,
                    "ID",                 "%s",       NX_DUMPER_TABLE_LEFT,
                    "Timestamp",          "%s",       NX_DUMPER_TABLE_LEFT,
                    "Last Checkpoint ID", "%" PRIu64, NX_DUMPER_TABLE_RIGHT,
                    NULL);

            if (apfs_modified_by_is_valid(&fs->apfs_formatted_by)) {
                nx_dumper_table_add(table,
                        "Formatted By",
                        fs->apfs_formatted_by.id,
                        _HTIME(nx_swap64(fs->apfs_formatted_by.timestamp)),
                        nx_swap64(fs->apfs_formatted_by.last_xid));
            }

            for (n = 0; n < sizeof(fs->apfs_modified_by) /
                    sizeof(fs->apfs_modified_by[0]); n++) {
                if (!apfs_modified_by_is_valid(&fs->apfs_modified_by[n]))
                    continue;

                nx_dumper_table_add(table,
                        "Modified By",
                        fs->apfs_modified_by[n].id,
                        _HTIME(nx_swap64(fs->apfs_modified_by[n].timestamp)),
                        nx_swap64(fs->apfs_modified_by[n].last_xid));
            }

            nx_dumper_emit_table(dumper, indent, table);
            nx_dumper_table_dispose(table);

            _DEC_INDENT();
        } else {
            nx_dumper_open_container(dumper, "authorship");

            if (apfs_modified_by_is_valid(&fs->apfs_formatted_by)) {
                nx_dumper_open_container(dumper, "formatted-by");
                nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                        "{e:id/%s}{e:timestamp/%s}{e:last-xid/%" PRIu64 "}",
                        fs->apfs_formatted_by.id,
                        _ITIME(nx_swap64(fs->apfs_formatted_by.timestamp)),
                        nx_swap64(fs->apfs_formatted_by.last_xid));
                nx_dumper_close(dumper);
            }

            if (!nx_dumper_is_xml(dumper)) {
                nx_dumper_open_list(dumper, "modified-by");
            }

            for (n = 0; n < sizeof(fs->apfs_modified_by) /
                    sizeof(fs->apfs_modified_by[0]); n++) {
                if (!apfs_modified_by_is_valid(&fs->apfs_modified_by[n]))
                    continue;

                if (nx_dumper_is_xml(dumper)) {
                    nx_dumper_attr(dumper, "index", "%" PRIuSIZE, n);
                    nx_dumper_open_container(dumper, "modified-by");
                } else {
                    nx_dumper_open_instance(dumper);
                }
                nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                        "{e:id/%s}{e:timestamp/%s}{e:last-xid/%" PRIu64 "}",
                        fs->apfs_modified_by[n].id,
                        _ITIME(nx_swap64(fs->apfs_modified_by[n].timestamp)),
                        nx_swap64(fs->apfs_modified_by[n].last_xid));
                nx_dumper_close(dumper);
            }

            if (!nx_dumper_is_xml(dumper)) {
                nx_dumper_close(dumper);
            }

            nx_dumper_close(dumper);
        }

        nx_dumper_emit(dumper, indent,
                _LABEL("Volume Name")
                "{d:/\"}{:vol-name/%s}{d:/\"}\n",
                fs->apfs_volname);
        nx_dumper_emit(dumper, indent,
                _LABEL("Next Document ID")
                "{:next-doc-id/%u}\n",
                nx_swap32(fs->apfs_next_doc_id));
        nx_dumper_emit(dumper, indent,
                _LABEL("Unknown Field 0x3C4")
                "{w:unknown-x3C4/%u}"
                "{d:/(%#x)}\n",
                nx_swap32(fs->apfs_unknown_x3C4),
                nx_swap32(fs->apfs_unknown_x3C4));
        nx_dumper_emit(dumper, indent,
                _LABEL("Unknown Field 0x3C8")
                "{w:unknown-x3C8/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(fs->apfs_unknown_x3C8),
                nx_swap64(fs->apfs_unknown_x3C8));
        nx_dumper_emit(dumper, indent,
                _LABEL("Crypt Flags?")
                "{dw:/%#" PRIx64 "}"
                "{e:crypt-flags/%" PRIu64 "}{d:/%s}\n",
                nx_swap64(fs->apfs_crypt_flags),
                nx_swap64(fs->apfs_crypt_flags),
                _bitflag_names(nx_swap64(fs->apfs_crypt_flags), NULL, NULL,
                    buf, sizeof(buf)));

        _DEC_INDENT();
    }
    nx_dumper_close(dumper);

    nx_dumper_flush(dumper);
}

#define _APFS_DSTREAM_DUMP(dstream, indent, label)                           \
do {                                                                         \
    char buf[64];                                                            \
                                                                             \
    nx_dumper_emit(dumper, indent, _TITLE("Data Stream") "\n");              \
    nx_dumper_open_container(dumper, "dstream");                             \
    {                                                                        \
        nx_dumper_emit(dumper, indent + 4,                                   \
                label("Size")                                                \
                "{w:size/%" PRIu64 "}"                                       \
                "{d:/(%#" PRIx64 ")}\n",                                     \
                nx_swap64((dstream)->dstream_size),                          \
                nx_swap64((dstream)->dstream_size));                         \
        nx_dumper_emit(dumper, indent + 4,                                   \
                label("Allocated Size")                                      \
                "{w:alloced-size/%" PRIu64 "}"                               \
                "{d:/(%#" PRIx64 ")}\n",                                     \
                nx_swap64((dstream)->dstream_alloced_size),                  \
                nx_swap64((dstream)->dstream_alloced_size));                 \
        nx_dumper_emit(dumper, indent + 4,                                   \
                label("Default Crypto ID")                                   \
                "{w:default-crypto-id/%" PRIu64 "}"                          \
                "{d:/(%#" PRIx64 ")}\n",                                     \
                nx_swap64((dstream)->dstream_default_crypto_id),             \
                nx_swap64((dstream)->dstream_default_crypto_id));            \
        nx_dumper_emit(dumper, indent + 4,                                   \
                label("UUID")                                                \
                "{:uuid/%s}\n",                                              \
                nx_uuid_format(&(dstream)->dstream_uuid, buf, sizeof(buf))); \
    }                                                                        \
    nx_dumper_close(dumper);                                                 \
} while (0)

static void
_apfs_xfield_dump_unknown(nx_dumper_t *dumper, void const *xdata, size_t xlen)
{
    size_t         size = xlen;
    uint8_t const *data = (uint8_t const *)xdata;

    if (nx_dumper_is_text(dumper)) {
        static char const *l0_format =
            _SUBLABEL4("Contents") "{d:/%s}\n";
        static char const *l1_format =
            _SPACE4("") "{d:/%s}\n";
        char buf[128];
        bool first = true;

        while (size != 0) {
            size_t len = _hexdump(data, size, buf, sizeof(buf));

            nx_dumper_emit(dumper, 8,
                    first ? l0_format : l1_format,
                    buf);

            data += len, size -= len, first = false;
        }
    } else {
        size_t  data64_len;
        char   *data64 = base64_encode(data, size, &data64_len, 0);

        if (data64 != NULL) {
            nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                    "{e:contents/%.*s}",
                    (int)data64_len, data64);
            free(data64);
        }
    }
}

static void
_apfs_xfield_dump(nx_dumper_t *dumper, apfs_xfield_t const *xf)
{
    size_t         n;
    char           buf[64];
    uint8_t const *data;

    data = (uint8_t const *)APFS_XFIELD_DATA(xf);

    nx_dumper_emit(dumper, 8, _TITLE("Extension Fields") "\n");

    nx_dumper_attr(dumper, "used-data", "%u", nx_swap16(xf->xf_used_data));
    nx_dumper_open_container(dumper, "xfields");
    {
        nx_dumper_emit(dumper, 12,
                _SUBLABEL3("Count")
                "{d:/%u}\n",
                nx_swap16(xf->xf_num_exts));
        if (!nx_dumper_is_xml(dumper)) {
            nx_dumper_emit(dumper, 12,
                    _SUBLABEL3("Used Data")
                    "{w:used-data/%u}"
                    "{d:/(%#x)}\n",
                    nx_swap16(xf->xf_used_data),
                    nx_swap16(xf->xf_used_data));
        }

        if (nx_dumper_is_xml(dumper)) {
            nx_dumper_open_container(dumper, "entries");
        } else {
            nx_dumper_open_list(dumper, "entries");
        }

        for (n = 0; n < nx_swap16(xf->xf_num_exts); n++) {
            size_t length = nx_swap16(xf->xf_entries[n].xf_len);

            char const *xfield_type_name = "Unknown";
            switch (xf->xf_entries[n].xf_type) {
                case APFS_DREC_EXT_SIBLING_ID:
                    xfield_type_name = "Directory Record Sibling ID";
                    break;

                case APFS_INO_EXT_TYPE_DOCUMENT_ID:
                    xfield_type_name = "Index Node Document ID";
                    break;

                case APFS_INO_EXT_TYPE_NAME:
                    xfield_type_name = "Index Node Name";
                    break;

                case APFS_INO_EXT_TYPE_DSTREAM:
                    xfield_type_name = "Index Node Data Stream";
                    break;

                case APFS_INO_EXT_TYPE_DIR_STATS_KEY:
                    xfield_type_name = "Index Node Directory Statistics Key";
                    break;

                case APFS_INO_EXT_TYPE_FS_UUID:
                    xfield_type_name = "Index Node File System UUID";
                    break;

                case APFS_INO_EXT_TYPE_SPARSE_BYTES:
                    xfield_type_name = "Index Node Sparse Bytes";
                    break;

                case APFS_INO_EXT_TYPE_DEVICE:
                    xfield_type_name = "Index Node Device Specifier";
                    break;
            }

            if (nx_dumper_is_text(dumper)) {
                nx_dumper_emit(dumper, 12, _TITLE("/Entry #%" PRIuSIZE) "\n", n);
            }
            if (nx_dumper_is_xml(dumper)) {
                nx_dumper_attr(dumper, "index", "%" PRIuSIZE, n);
                nx_dumper_attr(dumper, "type", "%u", xf->xf_entries[n].xf_type);
                nx_dumper_attr(dumper, "flags", "%u",
                        xf->xf_entries[n].xf_flags);
                nx_dumper_attr(dumper, "length", "%u",
                        nx_swap16(xf->xf_entries[n].xf_len));
                nx_dumper_open_container(dumper, "entry");
            } else {
                nx_dumper_open_instance(dumper);
                nx_dumper_emit(dumper, 16,
                        _SUBLABEL4("Type")
                        "{dw:/%#x}{e:type/%u}{d:/[%s]}\n",
                        xf->xf_entries[n].xf_type,
                        xf->xf_entries[n].xf_type,
                        xfield_type_name);
                nx_dumper_emit(dumper, 16,
                        _SUBLABEL4("Flags")
                        "{d:/%#x}{e:flags/%u}\n",
                        xf->xf_entries[n].xf_flags,
                        xf->xf_entries[n].xf_flags);
                nx_dumper_emit(dumper, 16,
                        _SUBLABEL4("Length")
                        "{w:length/%u}"
                        "{d:/(%#x)}\n",
                        nx_swap16(xf->xf_entries[n].xf_len),
                        nx_swap16(xf->xf_entries[n].xf_len));
            }

            switch (xf->xf_entries[n].xf_type) {
                case APFS_DREC_EXT_SIBLING_ID:
                    nx_dumper_emit(dumper, 16,
                            _SUBLABEL4("Sibling ID")
                            "{w:sibling-id/%" PRIu64 "}"
                            "{d:/(%#" PRIx64 ")}\n",
                            nx_swap64(*(uint64_t const *)data),
                            nx_swap64(*(uint64_t const *)data));
                    break;

                case APFS_INO_EXT_TYPE_DOCUMENT_ID:
                    nx_dumper_emit(dumper, 16,
                            _SUBLABEL4("Document ID")
                            "{:doc-id/%u}\n",
                            nx_swap32(*(uint32_t const *)data));
                    break;

                case APFS_INO_EXT_TYPE_NAME:
                    nx_dumper_emit(dumper, 16,
                            _SUBLABEL4("Name")
                            "{d:/\"}{:name/%.*s}{d:/\"}\n",
                            (int)length, (char const *)data);
                    break;

                case APFS_INO_EXT_TYPE_DSTREAM:
                    nx_dumper_set_indent(dumper,
                            nx_dumper_get_indent(dumper) + 12);
                    _APFS_DSTREAM_DUMP((apfs_dstream_t const *)data, 4,
                            _SUBLABEL5);
                    nx_dumper_set_indent(dumper,
                            nx_dumper_get_indent(dumper) - 12);
                    break;

                case APFS_INO_EXT_TYPE_DIR_STATS_KEY:
                    nx_dumper_emit(dumper, 16,
                            _SUBLABEL4("Directory Stats Key")
                            "{w:dir-stats-key/%" PRIu64 "}"
                            "{d:/(%#" PRIx64 ")}\n",
                            nx_swap64(*(uint64_t const *)data),
                            nx_swap64(*(uint64_t const *)data));
                    break;

                case APFS_INO_EXT_TYPE_FS_UUID:
                    nx_dumper_emit(dumper, 16,
                            _SUBLABEL4("File System UUID"),
                            "{:fs-uuid/%s}\n",
                            nx_uuid_format((nx_uuid_t const *)data,
                                buf, sizeof(buf)));
                    break;

                case APFS_INO_EXT_TYPE_SPARSE_BYTES:
                    nx_dumper_emit(dumper, 16,
                            _SUBLABEL4("Sparse Bytes"),
                            "{d:/%s}{e:sparse-bytes/%s}\n",
                            "Yes", "true");
                    break;

                case APFS_INO_EXT_TYPE_DEVICE:
                    {
                        uint32_t spec = nx_swap32(*(uint32_t const *)data);
                        nx_dumper_emit(dumper, 16,
                                _SUBLABEL4("Device")
                                "{d:/%u, %u}"
                                "{e:device-major/%u}{e:device-minor/%u}\n",
                                APFS_DEVICE_SPEC_MAJOR(spec),
                                APFS_DEVICE_SPEC_MINOR(spec),
                                APFS_DEVICE_SPEC_MAJOR(spec),
                                APFS_DEVICE_SPEC_MINOR(spec));
                    }
                    break;

                default:
                    _apfs_xfield_dump_unknown(dumper, data,
                            nx_swap16(xf->xf_entries[n].xf_len));
                    break;
            }

            nx_dumper_close(dumper);

            data += APFS_XFIELD_ALIGN_LENGTH(length);
        }

        nx_dumper_close(dumper);
    }
    nx_dumper_close(dumper);
}

void
apfs_snap_metadata_dump(nx_dumper_t *dumper,
        apfs_snap_metadata_key_t const *key,
        apfs_snap_metadata_value_t const *value)
{
    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS Snapshot Metadata") "\n");

    nx_dumper_open_container(dumper, "snap-metadata");

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Extent Ref Tree Object ID")
                "{w:extentref-tree-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->extentref_tree_oid),
                nx_swap64(value->extentref_tree_oid));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Snapshot Block Object ID")
                "{w:sblock-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->sblock_oid),
                nx_swap64(value->sblock_oid));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Unknown Field 0x10")
                "{w:unknown-x10/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap32(value->unknown_x10),
                nx_swap32(value->unknown_x10));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Unknown Field 0x18")
                "{w:unknown-x18/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap32(value->unknown_x18),
                nx_swap32(value->unknown_x18));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Index Node Count?")
                "{w:inum/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap32(value->inum),
                nx_swap32(value->inum));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Extent Ref Tree Type")
                "{d:/%#x}{e:extentref-tree-type/%u}\n",
                nx_swap32(value->extentref_tree_type),
                nx_swap32(value->extentref_tree_type));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Flags")
                "{d:/%#x}{e:flags/%u}\n",
                nx_swap32(value->flags),
                nx_swap32(value->flags));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Name")
                "{d:/\"}{:name/%.*s}{d:/\"}\n",
                (int)nx_swap16(value->name_len),
                (char const *)(value + 1));

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

void
apfs_object_extent_dump(nx_dumper_t *dumper,
        apfs_object_extent_key_t const *key,
        apfs_object_extent_value_t const *value)
{
    char buf[128];

    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS Object Extent") "\n");

    nx_dumper_open_container(dumper, "object-extent");

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Flags")
                "{dw:/%#x}{e:flags/%u}{d:/%s}\n",
                APFS_OBJECT_EXTENT_VALUE_FLAGS(value),
                APFS_OBJECT_EXTENT_VALUE_FLAGS(value),
                _bitflag_names(APFS_OBJECT_EXTENT_VALUE_FLAGS(value), NULL,
                    NULL, buf, sizeof(buf)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Length")
                "{w:length/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_EXTENT_VALUE_LENGTH(value),
                APFS_OBJECT_EXTENT_VALUE_LENGTH(value));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Unknown Field 0x10")
                "{w:unknown-x10/%u}"
                "{d:/(%#x)}\n",
                nx_swap32(value->unknown_x10),
                nx_swap32(value->unknown_x10));

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

static char const *
_format_mode(uint16_t mode, char *buf, size_t bufsiz)
{
    size_t      np;
    char       *p = buf;
    char const *type_name = "Unknown";

    switch (mode & APFS_INODE_MODE_IFMT) {
        case APFS_INODE_MODE_IFIFO:
            type_name = "FIFO";
            break;
        case APFS_INODE_MODE_IFCHR:
            type_name = "Character Device";
            break;
        case APFS_INODE_MODE_IFDIR:
            type_name = "Directory";
            break;
        case APFS_INODE_MODE_IFBLK:
            type_name = "Block Device";
            break;
        case APFS_INODE_MODE_IFREG:
            type_name = "Regular File";
            break;
        case APFS_INODE_MODE_IFLNK:
            type_name = "Symbolic Link";
            break;
        case APFS_INODE_MODE_IFSOCK:
            type_name = "Socket";
            break;
        case APFS_INODE_MODE_IFWHT:
            type_name = "Whiteout";
            break;
    }

    np = snprintf(p, bufsiz, "%s", type_name);
    p += np, bufsiz -= np;

    if (mode & 01000) {
        if (bufsiz > 1) {
            *p++ = ' ', bufsiz--;
        }

        np = snprintf(p, bufsiz, "%s", "VTX");
        p += np, bufsiz -= np;
    }

    if (mode & 07777) {
        size_t x = 0;
        if (bufsiz > 1) {
            *p++ = ' ', bufsiz--;
        }

        if (mode & 04700) {
            if (x++ != 0 && bufsiz > 1) {
                *p++ = ',', bufsiz--;
            }
            if (bufsiz > 1) {
                *p++ = 'u', bufsiz--;
            }
            if (bufsiz > 1) {
                *p++ = '+', bufsiz--;
            }
            if ((mode & 0400) != 0 && bufsiz > 1) {
                *p++ = 'r', bufsiz--;
            }
            if ((mode & 0200) != 0 && bufsiz > 1) {
                *p++ = 'w', bufsiz--;
            }
            if ((mode & 0100) != 0 && bufsiz > 1) {
                *p++ = (mode & 04000) ? 's' : 'x', bufsiz--;
            }
        }

        if (mode & 02070) {
            if (x++ != 0 && bufsiz > 1) {
                *p++ = ',', bufsiz--;
            }
            if (bufsiz > 1) {
                *p++ = 'g', bufsiz--;
            }
            if (bufsiz > 1) {
                *p++ = '+', bufsiz--;
            }
            if ((mode & 0040) != 0 && bufsiz > 1) {
                *p++ = 'r', bufsiz--;
            }
            if ((mode & 0020) != 0 && bufsiz > 1) {
                *p++ = 'w', bufsiz--;
            }
            if ((mode & 0010) != 0 && bufsiz > 1) {
                *p++ = (mode & 02000) ? 's' : 'x', bufsiz--;
            }
        }

        if (mode & 0007) {
            if (x++ != 0 && bufsiz > 1) {
                *p++ = ',', bufsiz--;
            }
            if (bufsiz > 1) {
                *p++ = 'o', bufsiz--;
            }
            if (bufsiz > 1) {
                *p++ = '+', bufsiz--;
            }
            if ((mode & 0004) != 0 && bufsiz > 1) {
                *p++ = 'r', bufsiz--;
            }
            if ((mode & 0002) != 0 && bufsiz > 1) {
                *p++ = 'w', bufsiz--;
            }
            if ((mode & 0001) != 0 && bufsiz > 1) {
                *p++ = 'x', bufsiz--;
            }
        }
    }

    *p = '\0';

    return buf;
}

void
apfs_inode_dump(nx_dumper_t *dumper,
        apfs_inode_key_t const *key,
        apfs_inode_value_t const *value,
        bool xfields)
{
    char buf[128];
    char timbuf1[64];
    char timbuf2[64];

    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS Index Node") "\n");

    nx_dumper_open_container(dumper, "inode");

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Parent ID")
                "{w:parent-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->parent_id),
                nx_swap64(value->parent_id));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("This ID")
                "{w:private-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->private_id),
                nx_swap64(value->private_id));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Creation Time")
                "{d:/%s}{e:creation-time/%s}\n",
                _HTIME(nx_swap64(value->creation_timestamp)),
                _ITIME(nx_swap64(value->creation_timestamp)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Modification Time")
                "{d:/%s}{e:modification-time/%s}\n",
                _HTIME(nx_swap64(value->modification_timestamp)),
                _ITIME(nx_swap64(value->modification_timestamp)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Changed Time")
                "{d:/%s}{e:changed-time/%s}\n",
                _HTIME(nx_swap64(value->changed_timestamp)),
                _ITIME(nx_swap64(value->changed_timestamp)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Access Time")
                "{d:/%s}{e:access-time/%s}\n",
                _HTIME(nx_swap64(value->access_timestamp)),
                _ITIME(nx_swap64(value->access_timestamp)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Internal Flags")
                "{dw:/%#" PRIx64 "}{e:internal-flags/%" PRIu64 "}{d:/%s}\n",
                nx_swap64(value->internal_flags),
                nx_swap64(value->internal_flags),
                _bitflag_names(nx_swap64(value->internal_flags), NULL,
                    NULL, buf, sizeof(buf)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Children Count")
                "{:nchildren/%u}\n",
                nx_swap32(value->nchildren));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Default Protection Class")
                "{:default-protection-class/%u}\n",
                nx_swap32(value->default_protection_class));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Unknown Field 0x40")
                "{w:unknown-x40/%u}"
                "{d:/(%#x)}\n",
                nx_swap32(value->unknown_x40),
                nx_swap32(value->unknown_x40));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("BSD Flags")
                "{dw:/%#x}{e:bsd-flags/%u}{d:/%s}\n",
                nx_swap32(value->bsd_flags),
                nx_swap32(value->bsd_flags),
                _bitflag_names(nx_swap32(value->bsd_flags), NULL,
                    "\001UF_NODUMP"
                    "\002UF_IMMUTABLE"
                    "\003UF_APPEND"
                    "\004UF_OPAQUE"
                    "\006UF_COMPRESSED"
                    "\007UF_TRACKED"
                    "\010UF_DATAVAULT"
                    "\020UF_HIDDEN"
                    "\021SF_SUPPORTED"
                    "\022SF_IMMUTABLE"
                    "\023SF_APPEND"
                    "\024SF_RESTRICTED"
                    "\025SF_NOUNLINK",
                    buf, sizeof(buf)));

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("User ID")
                "{:user-id/%u}\n",
                nx_swap32(value->user_id));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Group ID")
                "{:group-id/%u}\n",
                nx_swap32(value->group_id));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Mode")
                "{dw:/%07o}{e:mode/%u}{d:/[%s]}\n",
                nx_swap16(value->mode),
                nx_swap16(value->mode),
                _format_mode(nx_swap16(value->mode), buf, sizeof(buf)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Pad 1")
                "{w:pad1/%u}"
                "{d:/(%#x)}\n",
                nx_swap16(value->pad1),
                nx_swap16(value->pad1));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Pad 2")
                "{w:pad2/%u}"
                "{d:/(%#x)}\n",
                nx_swap64(value->pad2),
                nx_swap64(value->pad2));

        if (xfields) {
            _apfs_xfield_dump(dumper, (apfs_xfield_t const *)(value + 1));
        }

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

void
apfs_xattr_dump(nx_dumper_t *dumper,
        apfs_xattr_key_t const *key,
        apfs_xattr_value_t const *value)
{
    char buf[128];

    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS Extended Attribute") "\n");

    nx_dumper_open_container(dumper, "xattr");

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Name")
                "{d:/\"}{:name/%.*s}{d:/\"}\n",
                (int)nx_swap16(key->name_len),
                key->name);

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Flags")
                "{dw:/%#x}{e:flags/%u}{d:/%s}\n",
                nx_swap16(value->flags),
                nx_swap16(value->flags),
                _bitflag_names(nx_swap16(value->flags), NULL,
                    "\001INDIRECT\002INLINE\003SYMLINK",
                    buf, sizeof(buf)));
        if (nx_swap16(value->flags) & APFS_XATTR_VALUE_FLAG_INDIRECT) {
            apfs_xattr_indirect_value_t const *ivalue;

            ivalue = (apfs_xattr_indirect_value_t const *)value;

            nx_dumper_emit(dumper, 8,
                    _SUBLABEL2("Data Object ID")
                    "{w:xattr-obj-id/%" PRIu64 "}"
                    "{d:/(%#" PRIx64 ")}\n",
                    nx_swap64(ivalue->xattr_obj_id),
                    nx_swap64(ivalue->xattr_obj_id));

            _APFS_DSTREAM_DUMP(&ivalue->dstream, 4, _SUBLABEL2);
        } else {
            if (nx_swap16(value->flags) & APFS_XATTR_VALUE_FLAG_SYMLINK) {
                nx_dumper_emit(dumper, 8,
                        _SUBLABEL2("Symbolic Link Target")
                        "{d:/\"}{:xattr-symlink/%.*s}{d:/\"}\n",
                        (int)nx_swap16(value->xdata_len),
                        (char const *)(value + 1));
            } else {
                size_t         xsize = nx_swap16(value->xdata_len);
                uint8_t const *xdata = (uint8_t const *)(value + 1);

                nx_dumper_emit(dumper, 8,
                        _SUBLABEL2("Inline Data Length")
                        "{w:inline-xdata-len/%u}"
                        "{d:/(%#x)}\n",
                        nx_swap16(value->xdata_len),
                        nx_swap16(value->xdata_len));

                if (nx_dumper_is_text(dumper)) {
                    static char const *l0_format =
                        _SUBLABEL2("Inline Data Contents") "{d:/%s}\n";
                    static char const *l1_format =
                        _SPACE2("") "{d:/%s}\n";
                    char buf[128];
                    bool first = true;

                    while (xsize != 0) {
                        size_t len = _hexdump(xdata, xsize, buf, sizeof(buf));

                        nx_dumper_emit(dumper, 8,
                                first ? l0_format : l1_format,
                                buf);

                        xdata += len, xsize -= len, first = false;
                    }
                } else {
                    size_t  xdata64_len;
                    char   *xdata64 = base64_encode(xdata, xsize,
                            &xdata64_len, 0);

                    if (xdata64 != NULL) {
                        nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                                "{e:inline-xdata/%.*s}",
                                (int)xdata64_len, xdata64);
                        free(xdata64);
                    }
                }
            }
        }

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

void
apfs_sibling_dump(nx_dumper_t *dumper,
        apfs_sibling_key_t const *key,
        apfs_sibling_value_t const *value)
{
    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS Sibling") "\n");

    nx_dumper_open_container(dumper, "sibling");

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Sibling ID")
                "{w:sibling-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(key->sibling_id),
                nx_swap64(key->sibling_id));

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Parent ID")
                "{w:parent-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->parent_id),
                nx_swap64(value->parent_id));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Name")
                "{d:/\"}{:name/%.*s}{d:/\"}\n",
                (int)nx_swap16(value->name_len),
                (char const *)(value + 1));

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

void
apfs_object_dstream_dump(nx_dumper_t *dumper,
        apfs_object_dstream_key_t const *key,
        apfs_object_dstream_value_t const *value)
{
    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS Object Data Stream") "\n");

    nx_dumper_open_container(dumper, "object-dstream");

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Count")
                "{:count/%u}\n",
                nx_swap32(value->count));

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

void
apfs_crypto_dump(nx_dumper_t *dumper,
        apfs_crypto_key_t const *key,
        apfs_crypto_value_t const *value)
{
    char buf[128];

    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS Crypto") "\n");

    nx_dumper_open_container(dumper, "crypto");

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8, _TITLE("State") "\n");
        nx_dumper_open_container(dumper, "state");
        {
            nx_dumper_emit(dumper, 12,
                    _SUBLABEL3("Version")
                    "{d:/%u.%u}{e:version-major/%u}{e:version-minor/%u}\n",
                    nx_swap16(value->state.major_version),
                    nx_swap16(value->state.minor_version));
            nx_dumper_emit(dumper, 12,
                    _SUBLABEL3("Crypto Flags")
                    "{dw:/%#x}{e:cp-flags/%u}{d:/%s}\n",
                    nx_swap32(value->state.cpflags),
                    nx_swap32(value->state.cpflags),
                    _bitflag_names(nx_swap32(value->state.cpflags), NULL, NULL,
                        buf, sizeof(buf)));
            nx_dumper_emit(dumper, 12,
                    _SUBLABEL3("Persistent Class")
                    "{d:/%#x}{e:persistent-class/%u}\n",
                    nx_swap32(value->state.persistent_class),
                    nx_swap32(value->state.persistent_class));
            nx_dumper_emit(dumper, 12,
                    _SUBLABEL3("Key OS Version")
                    "{d:/%#x}{e:key-os-version/%u}\n",
                    nx_swap32(value->state.key_os_version),
                    nx_swap32(value->state.key_os_version));
            nx_dumper_emit(dumper, 12,
                    _SUBLABEL3("Key Revision")
                    "{d:/%#x}{e:key-revision/%u}\n",
                    nx_swap16(value->state.key_revision),
                    nx_swap16(value->state.key_revision));
            nx_dumper_emit(dumper, 12,
                    _SUBLABEL3("Key Length")
                    "{d:/%#x}{e:key-length/%u}\n",
                    nx_swap16(value->state.key_len),
                    nx_swap16(value->state.key_len));

            {
                size_t         key_size = nx_swap16(value->state.key_len);
                uint8_t const *key_data = (uint8_t const *)(value + 1);

                if (nx_dumper_is_text(dumper)) {
                    static char const *l0_format =
                        _SUBLABEL3("Key Material") "{d:/%s}\n";
                    static char const *l1_format =
                        _SPACE3("") "{d:/%s}\n";
                    char buf[128];
                    bool first = true;

                    while (key_size != 0) {
                        size_t len = _hexdump(key_data, key_size,
                                buf, sizeof(buf));

                        nx_dumper_emit(dumper, 12,
                                first ? l0_format : l1_format,
                                buf);

                        key_data += len, key_size -= len, first = false;
                    }
                } else {
                    size_t  key_data64_len;
                    char   *key_data64 = base64_encode(key_data, key_size,
                            &key_data64_len, 0);

                    if (key_data64 != NULL) {
                        nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                                "{e:key-material/%.*s}",
                                (int)key_data64_len, key_data64);
                        free(key_data64);
                    }
                }
            }
        }
        nx_dumper_close(dumper);

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Reference Count")
                "{:reference-count/%u}\n",
                nx_swap32(value->refcnt));

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

void
apfs_file_extent_dump(nx_dumper_t *dumper,
        apfs_file_extent_key_t const *key,
        apfs_file_extent_value_t const *value)
{
    char buf[128];

    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS File Extent") "\n");

    nx_dumper_open_container(dumper, "file-extent");

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Offset")
                "{w:offset/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(key->offset),
                nx_swap64(key->offset));

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Flags")
                "{dw:/%#x}{e:flags/%u}{d:/%s}\n",
                APFS_FILE_EXTENT_VALUE_FLAGS(value),
                APFS_FILE_EXTENT_VALUE_FLAGS(value),
                _bitflag_names(APFS_FILE_EXTENT_VALUE_FLAGS(value), NULL,
                    "\001NOCRYPTO", buf, sizeof(buf)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Length")
                "{w:length/%" PRIu64 "}"
                "{dw:/(%#" PRIx64 ")}"
                "{d:/[%" PRIu64 " Blocks]}\n",
                APFS_FILE_EXTENT_VALUE_LENGTH(value),
                APFS_FILE_EXTENT_VALUE_LENGTH(value),
                APFS_FILE_EXTENT_VALUE_LENGTH(value) / NX_OBJECT_SIZE);
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Physical Block #")
                "{w:phys-block-num/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->phys_block_num),
                nx_swap64(value->phys_block_num));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Crypto ID")
                "{w:crypto-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->crypto_id),
                nx_swap64(value->crypto_id));

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

void
apfs_drec_dump(nx_dumper_t *dumper,
        apfs_drec_key_t const *key,
        apfs_drec_value_t const *value,
        bool xfields)
{
    char     buf[128];
    char     timbuf1[64];
    char     timbuf2[64];
    uint32_t hash;

    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS Directory Record") "\n");

    nx_dumper_open_container(dumper, "drec");

    if (key != NULL) {
        /* Compute the hash to verify */
        hash = apfs_hash_name(key->hashed.name,
                APFS_DREC_HASHED_NAME_LENGTH(key) - 1,
                true);

        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Name")
                "{d:/\"}{:name/%.*s}{d:/\"}\n",
                (int)(APFS_DREC_HASHED_NAME_LENGTH(key) - 1),
                key->hashed.name);
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Hash")
                "{dw:/%#x}{e:hash/%u}{d:/[%s]}\n",
                APFS_DREC_HASHED_NAME_HASH(key),
                APFS_DREC_HASHED_NAME_HASH(key),
                APFS_DREC_HASHED_NAME_HASH(key) == hash ? "Valid" : "Invalid");

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        char const *item_type_name = "Unknown";

        switch (APFS_DREC_VALUE_ITEM_TYPE(value)) {
            case APFS_ITEM_TYPE_FIFO:
                item_type_name = "FIFO";
                break;
            case APFS_ITEM_TYPE_CHAR_DEVICE:
                item_type_name = "Character Device";
                break;
            case APFS_ITEM_TYPE_DIRECTORY:
                item_type_name = "Directory";
                break;
            case APFS_ITEM_TYPE_BLOCK_DEVICE:
                item_type_name = "Block Device";
                break;
            case APFS_ITEM_TYPE_REGULAR:
                item_type_name = "Regular File";
                break;
            case APFS_ITEM_TYPE_SYMBOLIC_LINK:
                item_type_name = "Symbolic Link";
                break;
            case APFS_ITEM_TYPE_SOCKET:
                item_type_name = "Socket";
                break;
            case APFS_ITEM_TYPE_WHITEOUT:
                item_type_name = "Whiteout";
                break;
        }

        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("File ID")
                "{w:file-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->file_id),
                nx_swap64(value->file_id));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Timestamp")
                "{d:/%s}{e:timestamp/%s}\n",
                _HTIME(nx_swap64(value->timestamp)),
                _ITIME(nx_swap64(value->timestamp)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Flags")
                "{dw:/%#x}{e:flags/%u}{d:/%s}\n",
                APFS_DREC_VALUE_FLAGS(value),
                APFS_DREC_VALUE_FLAGS(value),
                _bitflag_names(APFS_DREC_VALUE_FLAGS(value), NULL,
                    "\006SPARSE", buf, sizeof(buf)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Item Type")
                "{dw:/%u}{e:item-type/%u}{d:/[%s]}\n",
                APFS_DREC_VALUE_ITEM_TYPE(value),
                APFS_DREC_VALUE_ITEM_TYPE(value),
                item_type_name);

        if (xfields) {
            _apfs_xfield_dump(dumper, (apfs_xfield_t const *)(value + 1));
        }

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

void
apfs_dirstats_dump(nx_dumper_t *dumper,
        apfs_dirstats_key_t const *key,
        apfs_dirstats_value_t const *value)
{
    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS Directory Statistics") "\n");
    nx_dumper_open_container(dumper, "dirstats");

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Unknown Field 0x0")
                "{w:unknown-x0/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->unknown_x0),
                nx_swap64(value->unknown_x0));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Unknown Field 0x8")
                "{w:unknown-x8/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->unknown_x8),
                nx_swap64(value->unknown_x8));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Chained Key")
                "{w:chained-key/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->chained_key),
                nx_swap64(value->chained_key));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Generation Count")
                "{:gen-count/%" PRIu64 "}\n",
                nx_swap64(value->gen_count));

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

void
apfs_snap_name_dump(nx_dumper_t *dumper,
        apfs_snap_name_key_t const *key,
        apfs_snap_name_value_t const *value)
{
    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS Snapshot Name") "\n");
    nx_dumper_open_container(dumper, "snap-name");

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));
        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Snapshot Name")
                "{d:/\"}{:snap-name/%.*s}{d:/\"}\n",
                (int)nx_swap16(key->name_len),
                (char const *)(key + 1));

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Snapshot Checkpoint ID")
                "{w:snap-xid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->snap_xid),
                nx_swap64(value->snap_xid));

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

void
apfs_sibling_map_dump(nx_dumper_t *dumper,
        apfs_sibling_map_key_t const *key,
        apfs_sibling_map_value_t const *value)
{
    if (dumper == NULL || (key == NULL && value == NULL))
        return;

    nx_dumper_emit(dumper, 0, _TITLE("APFS Sibling Map") "\n");
    nx_dumper_open_container(dumper, "sibling-map");

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)),
                APFS_OBJECT_ID_ID(nx_swap64(key->obj_id)));

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("File ID")
                "{w:file-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(value->file_id));

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

static void
apfs_unknown_dump(nx_dumper_t *dumper,
        void const *key, size_t key_len,
        void const *value, size_t value_len)
{
    uint64_t obj_id = nx_swap64(*(uint64_t const *)key);

    nx_dumper_emit(dumper, 0, _TITLE("/APFS Unknown Object Type %#x") "\n",
            APFS_OBJECT_ID_TYPE(obj_id));

    nx_dumper_attr(dumper, "type", "%u", APFS_OBJECT_ID_TYPE(obj_id));
    nx_dumper_open_container(dumper, "unknown-object");

    if (!nx_dumper_is_xml(dumper)) {
        nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                "{e:type/%u}", APFS_OBJECT_ID_TYPE(obj_id));
    }

    if (key != NULL) {
        nx_dumper_emit(dumper, 4, _TITLE("Key") "\n");
        nx_dumper_open_container(dumper, "key");

        nx_dumper_emit(dumper, 8,
                _SUBLABEL2("Object ID")
                "{w:obj-id/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                APFS_OBJECT_ID_ID(obj_id),
                APFS_OBJECT_ID_ID(obj_id));

        if (key_len > sizeof(uint64_t)) {
            size_t         size = key_len - sizeof(uint64_t);
            uint8_t const *data = (uint8_t const *)key + sizeof(uint64_t);

            if (nx_dumper_is_text(dumper)) {
                static char const *l0_format =
                    _SUBLABEL2("Contents") "{d:/%s}\n";
                static char const *l1_format =
                    _SPACE2("") "{d:/%s}\n";
                char buf[128];
                bool first = true;

                while (size != 0) {
                    size_t len = _hexdump(data, size, buf, sizeof(buf));

                    nx_dumper_emit(dumper, 8,
                            first ? l0_format : l1_format,
                            buf);

                    data += len, size -= len, first = false;
                }
            } else {
                size_t  data64_len;
                char   *data64 = base64_encode(data, size, &data64_len, 0);

                if (data64 != NULL) {
                    nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                            "{e:contents/%.*s}",
                            (int)data64_len, data64);
                    free(data64);
                }
            }
        }

        nx_dumper_close(dumper);
    }

    if (value != NULL) {
        size_t         size = value_len;
        uint8_t const *data = (uint8_t const *)value;

        nx_dumper_emit(dumper, 4, _TITLE("Value") "\n");
        nx_dumper_open_container(dumper, "value");

        if (nx_dumper_is_text(dumper)) {
            static char const *l0_format =
                _SUBLABEL2("Contents") "{d:/%s}\n";
            static char const *l1_format =
                _SPACE2("") "{d:/%s}\n";
            char buf[128];
            bool first = true;

            while (size != 0) {
                size_t len = _hexdump(data, size, buf, sizeof(buf));

                nx_dumper_emit(dumper, 8,
                        first ? l0_format : l1_format,
                        buf);

                data += len, size -= len, first = false;
            }
        } else {
            size_t  data64_len;
            char   *data64 = base64_encode(data, size, &data64_len, 0);

            if (data64 != NULL) {
                nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                        "{e:contents/%.*s}",
                        (int)data64_len, data64);
                free(data64);
            }
        }

        nx_dumper_close(dumper);
    }

    nx_dumper_close(dumper);
}

void
apfs_object_dump(nx_dumper_t *dumper,
        void const *key, size_t key_len,
        void const *value, size_t value_len)
{
    uint64_t obj_id;

    if (dumper == NULL)
        return;

    if (key == NULL || key_len < sizeof(uint64_t))
        return;
    if (value == NULL)
        return;

    obj_id = nx_swap64(*(uint64_t const *)key);

#define CALL_DUMPER(name) \
    apfs_##name##_dump(dumper, \
            (apfs_##name##_key_t const *)(key), \
            (apfs_##name##_value_t const *)(value))
#define CALL_DUMPER2(name) \
    apfs_##name##_dump(dumper, \
            (apfs_##name##_key_t const *)(key), \
            (apfs_##name##_value_t const *)(value), \
            value_len > sizeof(apfs_##name##_value_t))

    switch (APFS_OBJECT_ID_TYPE(obj_id)) {
        case APFS_OBJECT_TYPE_SNAP_METADATA:
            CALL_DUMPER(snap_metadata);
            break;

        case APFS_OBJECT_TYPE_OBJECT_EXTENT:
            CALL_DUMPER(object_extent);
            break;

        case APFS_OBJECT_TYPE_INODE:
            CALL_DUMPER2(inode);
            break;

        case APFS_OBJECT_TYPE_XATTR:
            CALL_DUMPER(xattr);
            break;

        case APFS_OBJECT_TYPE_SIBLING:
            CALL_DUMPER(sibling);
            break;

        case APFS_OBJECT_TYPE_OBJECT_DSTREAM:
            CALL_DUMPER(object_dstream);
            break;

        case APFS_OBJECT_TYPE_CRYPTO:
            CALL_DUMPER(crypto);
            break;

        case APFS_OBJECT_TYPE_FILE_EXTENT:
            CALL_DUMPER(file_extent);
            break;

        case APFS_OBJECT_TYPE_DREC:
            CALL_DUMPER2(drec);
            break;

        case APFS_OBJECT_TYPE_DIRSTATS:
            CALL_DUMPER(dirstats);
            break;

        case APFS_OBJECT_TYPE_SNAP_NAME:
            CALL_DUMPER(snap_name);
            break;

        case APFS_OBJECT_TYPE_SIBLING_MAP:
            CALL_DUMPER(sibling_map);
            break;

        default:
            apfs_unknown_dump(dumper, key, key_len, value, value_len);
            break;
    }
}
