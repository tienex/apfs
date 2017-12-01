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
#include "nx/format/apfs.h"

#include "nxcompat/nxcompat.h"

#include <stdlib.h>

#include "formatting.h"

char const *
nx_object_type_name_r(uint16_t type, uint32_t subtype, char *buf, size_t bufsiz)
{
    char const * const tree_objects[] = {
        "object map", NULL, NULL, "apfs root",
        "apfs extent ref", "apfs snap meta", NULL, NULL,
        "object map snapshot", NULL, NULL, NULL
    };

    switch (type) {
        case NX_OBJECT_TYPE_CONTAINER:
            return "nx container";
        case NX_OBJECT_TYPE_BTREE_ROOT:
        case NX_OBJECT_TYPE_BTREE_NODE:
            if (buf != NULL && bufsiz > 0) {
                if (subtype >= 0xb && subtype <= 0x13) {
                    char const *objtype = tree_objects[subtype - 0xb];
                    if (objtype != NULL) {
                        snprintf(buf, bufsiz, "%s: btree %s", objtype,
                                (type == NX_OBJECT_TYPE_BTREE_ROOT) ?
                                "root" : "node");

                        return buf;
                    }
                }
            }

            return (type == NX_OBJECT_TYPE_BTREE_ROOT) ?
                "btree root" : "btree node";
        case NX_OBJECT_TYPE_MIDDLE_TREE:
            if (buf != NULL && bufsiz > 0) {
                if (subtype >= 0xb && subtype <= 0x13) {
                    char const *objtype = tree_objects[subtype - 0xb];
                    if (objtype != NULL) {
                        snprintf(buf, bufsiz, "%s: middle tree", objtype);
                        return buf;
                    }
                }
            }

            return "middle tree";
        case NX_OBJECT_TYPE_SPACEMAN:
            return "space manager";
        case NX_OBJECT_TYPE_CAB:
            return "chunk allocation block";
        case NX_OBJECT_TYPE_CIB:
            return "chunk information block";
        case NX_OBJECT_TYPE_SPACEMAN_IP:
            return "space manager internal pool";
        case NX_OBJECT_TYPE_OBJECT_MAP:
            return "object map";
        case NX_OBJECT_TYPE_CHECKPOINT_MAP:
            return "checkpoint map";
        case NX_OBJECT_TYPE_APFS_VOLUME:
            return "apfs volume";
        case NX_OBJECT_TYPE_NEXT_REAP:
            return "next reap";
        case NX_OBJECT_TYPE_NEXT_REAP_LIST:
            return "next reap list";
        case NX_OBJECT_TYPE_WBC:
            return "write-back chunk";
        case NX_OBJECT_TYPE_WBC_LIST:
            return "write-back chunk list";
        default:
            return "?";
    }
}

char const *
nx_object_type_name(uint16_t type, uint32_t subtype)
{
    static __thread char buf[64];
    return nx_object_type_name_r(type, subtype, buf, sizeof(buf));
}

char const *
nx_object_name_r(nx_object_t const *object, char *buf, size_t bufsiz)
{
    return nx_object_type_name_r(nx_swap32(object->o_type),
            nx_swap32(object->o_subtype), buf, bufsiz);
}

char const *
nx_object_name(nx_object_t const *object)
{
    return nx_object_type_name(nx_swap32(object->o_type),
            nx_swap32(object->o_subtype));
}

char const *
nx_uuid_format(nx_uuid_t const *uuid, char *buf, size_t bufsiz)
{
    if (uuid == NULL || buf == NULL)
        return NULL;

    snprintf(buf, bufsiz,
             "%02X%02X%02X%0X-%02X%02X-%02X%02X-%02X%02X-"
             "%02X%02X%02X%02X%02X%02X",
             uuid->bytes[0], uuid->bytes[1], uuid->bytes[2], uuid->bytes[3],
             uuid->bytes[4], uuid->bytes[5], uuid->bytes[6], uuid->bytes[7],
             uuid->bytes[8], uuid->bytes[9], uuid->bytes[10], uuid->bytes[11],
             uuid->bytes[12], uuid->bytes[13], uuid->bytes[14], uuid->bytes[15]);

    return buf;
}

static char const *
_objectref_name(uint32_t type)
{
    return ((type & NX_OBJECT_FLAG_DIRECT) ? "Direct" :
            (type & NX_OBJECT_FLAG_CHECKPOINT) ? "Checkpoint" : "Object Map");
}

void
nx_super_dump(nx_dumper_t *dumper, nx_super_t const *sb)
{
    size_t n, indent = 0;
    char   buf[128];

    if (dumper == NULL || sb == NULL)
        return;

    nx_dumper_emit(dumper, indent, _TITLE("NX Super") "\n");

    _nx_object_dump0(dumper, NX_OBJECT(sb));
    nx_dumper_open_container(dumper, "nx-super");
    {
        _INC_INDENT();

        _nx_object_dump1(dumper, indent, NX_OBJECT(sb));
        nx_dumper_emit(dumper, indent,
                _LABEL("Signature")
                "{:signature/%4.4s}\n",
                (char const *)&sb->nx_signature);
        nx_dumper_emit(dumper, indent,
                _LABEL("Block Size")
                "{:block-size/%u}\n",
                nx_swap32(sb->nx_block_size));
        nx_dumper_emit(dumper, indent,
                _LABEL("Block Count")
                "{w:block-count/%" PRIu64 "}"
                "{d:/(}{h,hn-1000,hn-decimal:bytes-capacity/%" PRIu64 "}{d:/)}"
                "\n",
                nx_swap64(sb->nx_block_count),
                nx_swap32(sb->nx_block_size) * nx_swap64(sb->nx_block_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("Features")
                "{dw:/%#" PRIx64 "}"
                "{e:features/%" PRIu64 "}{d:/%s}\n",
                nx_swap64(sb->nx_features),
                nx_swap64(sb->nx_features),
                _bitflag_names(nx_swap64(sb->nx_features), NULL, NULL,
                    buf, sizeof(buf)));
        nx_dumper_emit(dumper, indent,
                _LABEL("Read Only Compatible Features")
                "{dw:/%#" PRIx64 "}"
                "{e:ro-compatible-features/%#" PRIu64 "}{d:/%s}\n",
                nx_swap64(sb->nx_readonly_compatible_features),
                nx_swap64(sb->nx_readonly_compatible_features),
                _bitflag_names(nx_swap64(sb->nx_readonly_compatible_features),
                    NULL, NULL, buf, sizeof(buf)));
        nx_dumper_emit(dumper, indent,
                _LABEL("Incompatible Features")
                "{dw:/%#" PRIx64 "}"
                "{e:incompatible-features/%" PRIu64 "}{d:/%s}\n",
                nx_swap64(sb->nx_incompatible_features),
                nx_swap64(sb->nx_incompatible_features),
                _bitflag_names(nx_swap64(sb->nx_incompatible_features),
                    NULL, NULL, buf, sizeof(buf)));
        nx_dumper_emit(dumper, indent,
                _LABEL("UUID")
                "{:uuid}\n",
                nx_uuid_format(&sb->nx_uuid, buf, sizeof(buf)));
        nx_dumper_emit(dumper, indent,
                _LABEL("Next Object ID")
                "{w:next-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(sb->nx_next_oid),
                nx_swap64(sb->nx_next_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Next Checkpoint ID")
                "{w:next-xid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(sb->nx_next_xid),
                nx_swap64(sb->nx_next_xid));
        nx_dumper_open_container(dumper, "checkpoint");
        {
            nx_dumper_emit(dumper, indent,
                    _TITLE("Checkpoint Descriptors") "\n");

            nx_dumper_open_container(dumper, "descriptors");
            {
                _INC_INDENT();
                nx_dumper_emit(dumper, indent,
                        _SUBLABEL("Block Count")
                        "{:block-count/%u}\n",
                        nx_swap32(sb->nx_xp_desc_blocks));
                nx_dumper_emit(dumper, indent,
                        _SUBLABEL("First Block")
                        "{w:first-block/%" PRIu64 "}"
                        "{d:/(%#" PRIx64 ")}\n",
                        nx_swap64(sb->nx_xp_desc_first),
                        nx_swap64(sb->nx_xp_desc_first));
                nx_dumper_emit(dumper, indent,
                        _SUBLABEL("Next Index")
                        "{:next/%u}\n",
                        nx_swap32(sb->nx_xp_desc_next));
                nx_dumper_emit(dumper, indent,
                        _SUBLABEL("Current Index")
                        "{:index/%u}\n",
                        nx_swap32(sb->nx_xp_desc_index));
                nx_dumper_emit(dumper, indent,
                        _SUBLABEL("Count")
                        "{:length/%u}\n",
                        nx_swap32(sb->nx_xp_desc_len) + 1);
                _DEC_INDENT();
            }
            nx_dumper_close(dumper);

            nx_dumper_emit(dumper, indent, _TITLE("Checkpoint Data") "\n");

            nx_dumper_open_container(dumper, "data");
            {
                _INC_INDENT();
                nx_dumper_emit(dumper, indent,
                        _SUBLABEL("Block Count")
                        "{:block-count/%u}\n",
                        nx_swap32(sb->nx_xp_data_blocks));
                nx_dumper_emit(dumper, indent,
                        _SUBLABEL("First Block")
                        "{w:first-block/%" PRIu64 "}"
                        "{d:/(%#" PRIx64 ")}\n",
                        nx_swap64(sb->nx_xp_data_first),
                        nx_swap64(sb->nx_xp_data_first));
                nx_dumper_emit(dumper, indent,
                        _SUBLABEL("Next Index")
                        "{:next/%u}\n",
                        nx_swap32(sb->nx_xp_data_next));
                nx_dumper_emit(dumper, indent,
                        _SUBLABEL("Current Index")
                        "{:index/%u}\n",
                        nx_swap32(sb->nx_xp_data_index));
                nx_dumper_emit(dumper, indent,
                        _SUBLABEL("Count")
                        "{:length/%u}\n",
                        nx_swap32(sb->nx_xp_data_len) + 1);
                _DEC_INDENT();
            }
            nx_dumper_close(dumper);
        }
        nx_dumper_close(dumper);
        nx_dumper_emit(dumper, indent,
                _LABEL("Space Manager Object ID")
                "{w:spaceman-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(sb->nx_spaceman_oid),
                nx_swap64(sb->nx_spaceman_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Object Map Block #")
                "{w:omap-block/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(sb->nx_omap_oid),
                nx_swap64(sb->nx_omap_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Reaper Object ID")
                "{w:reaper-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(sb->nx_reaper_oid),
                nx_swap64(sb->nx_reaper_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Maximum # of File Systems")
                "{:max-file-systems/%u}\n",
                nx_swap32(sb->nx_max_file_systems));

        nx_dumper_emit(dumper, indent, _TITLE("File Systems") "\n");

        nx_dumper_open_list(dumper, "file-systems");
        {
            _INC_INDENT();
            for (n = 0; n < nx_swap32(sb->nx_max_file_systems) &&
                    n < sizeof(sb->nx_fs_oid) / sizeof(sb->nx_fs_oid[0]); n++) {
                if (sb->nx_fs_oid[n] != 0) {
                    nx_dumper_open_instance(dumper);
                    {
                        nx_dumper_attr(dumper, "index", "%" PRIuSIZE, n);
                        nx_dumper_emit(dumper, indent,
                                _SUBLABELx("{dw:/%3" PRIuSIZE ".}", "Object ID")
                                "{w:oid/%" PRIu64 "}"
                                "{d:/(%#" PRIx64 ")}\n",
                                n + 1,
                                nx_swap64(sb->nx_fs_oid[n]),
                                nx_swap64(sb->nx_fs_oid[n]));
                    }
                    nx_dumper_close(dumper);
                }
            }
            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        nx_dumper_emit(dumper, indent, _TITLE("Blocked Out") "\n");

        nx_dumper_open_container(dumper, "blocked-out");
        {
            _INC_INDENT();
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Length")
                    "{w:length/%" PRIu64 "}"
                    "{d:/(%#" PRIx64 ")}\n",
                    nx_swap64(sb->nx_blocked_out_len),
                    nx_swap64(sb->nx_blocked_out_len));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("First")
                    "{w:first/%" PRIu64 "}"
                    "{d:/(%#" PRIx64 ")}\n",
                    nx_swap64(sb->nx_blocked_out),
                    nx_swap64(sb->nx_blocked_out));
            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        nx_dumper_emit(dumper, indent, _TITLE("Key Bag Data") "\n");

        nx_dumper_open_container(dumper, "key-bag-data");
        {
            _INC_INDENT();
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Length")
                    "{w:length/%" PRIu64 "}"
                    "{d:/(%#" PRIx64 ")}\n",
                    nx_swap64(sb->nx_keybag_data_len),
                    nx_swap64(sb->nx_keybag_data_len));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("First")
                    "{w:first/%" PRIu64 "}"
                    "{d:/(%#" PRIx64 ")}\n",
                    nx_swap64(sb->nx_keybag_data),
                    nx_swap64(sb->nx_keybag_data));
            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        nx_dumper_emit(dumper, indent, _TITLE("Ephemeral Information") "\n");

        nx_dumper_open_list(dumper, "ephemeral-info");
        {
            _INC_INDENT();
            for (n = 0; n < sizeof(sb->nx_ephemeral_info) /
                    sizeof(sb->nx_ephemeral_info[0]); n++) {
                nx_dumper_open_instance(dumper);
                {
                    char buf[128];

                    nx_dumper_attr(dumper, "index", "%" PRIuSIZE, n);
                    nx_dumper_emit(dumper, indent,
                            _SUBLABELx("{dw:/%3" PRIuSIZE ".}", "Flags")
                            "{dw:/%#" PRIx64 "}"
                            "{e:flags/%" PRIu64 "}{d:/%s}\n",
                            n + 1,
                            nx_swap64(sb->nx_ephemeral_info[n]),
                            nx_swap64(sb->nx_ephemeral_info[n]),
                            _bitflag_names(nx_swap64(sb->nx_ephemeral_info[n]),
                                NULL, NULL, buf, sizeof(buf)));
                }
                nx_dumper_close(dumper);
            }
            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        nx_dumper_emit(dumper, indent,
                _LABEL("Fusion MT Object ID")
                "{w:fusion-mt-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 "})\n",
                nx_swap64(sb->nx_fusion_mt_oid),
                nx_swap64(sb->nx_fusion_mt_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Fusion WBC Object ID")
                "{w:fusion-wbc-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(sb->nx_fusion_wbc_oid),
                nx_swap64(sb->nx_fusion_wbc_oid));

        nx_dumper_emit(dumper, indent, _TITLE("Fusion WBC") "\n");

        nx_dumper_open_container(dumper, "fusion-wbc");
        {
            _INC_INDENT();
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("First")
                    "{w:first/%" PRIu64 "}"
                    "{d:/(%#" PRIx64 ")}\n",
                    nx_swap64(sb->nx_fusion_wbc),
                    nx_swap64(sb->nx_fusion_wbc));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Length")
                    "{w:length/%" PRIu64 "}"
                    "{d:/(%#" PRIx64 ")}\n",
                    nx_swap64(sb->nx_fusion_wbc_len),
                    nx_swap64(sb->nx_fusion_wbc_len));
            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        _DEC_INDENT();
    }
    nx_dumper_close(dumper);

    nx_dumper_flush(dumper);
}

void
nx_efi_jumpstart_dump(nx_dumper_t *dumper, nx_efi_jumpstart_t const *ejs)
{
    size_t indent = 0;

    if (dumper == NULL || ejs == NULL)
        return;

    nx_dumper_emit(dumper, indent, _TITLE("NX EFI Jump Start Record") "\n");

    _nx_object_dump0(dumper, NX_OBJECT(ejs));
    nx_dumper_open_container(dumper, "efi-jumpstart-record");
    {
        _INC_INDENT();

        _nx_object_dump1(dumper, indent, NX_OBJECT(ejs));
        nx_dumper_emit(dumper, indent,
                _LABEL("Signature")
                "{:signature/%4.4s}\n",
                (char const *)&ejs->ejs_signature);
        nx_dumper_emit(dumper, indent,
                _LABEL("Version")
                "{:version/%u}\n",
                nx_swap32(ejs->ejs_signature));
        nx_dumper_emit(dumper, indent,
                _LABEL("File Length")
                "{:file-length/%u}\n",
                nx_swap64(ejs->ejs_file_length));
        nx_dumper_emit(dumper, indent,
                _LABEL("Extent Count")
                "{:extent-count/%u}\n",
                nx_swap32(ejs->ejs_extent_count));

        _DEC_INDENT();
    }
    nx_dumper_close(dumper);

    nx_dumper_flush(dumper);
}

void
nx_cib_dump(nx_dumper_t *dumper, nx_cib_t const *cib)
{
    size_t n, indent = 0;
    char   buf[128];

    if (dumper == NULL || cib == NULL)
        return;

    nx_dumper_emit(dumper, indent, _TITLE("NX Chunk Information Block") "\n");

    _nx_object_dump0(dumper, NX_OBJECT(cib));
    nx_dumper_attr(dumper, "flags", "%u", nx_swap32(cib->cib_flags));
    nx_dumper_attr(dumper, "count", "%u", nx_swap32(cib->cib_count));

    nx_dumper_open_container(dumper, "cib");
    {
        _INC_INDENT();

        _nx_object_dump1(dumper, indent, NX_OBJECT(cib));
        if (!nx_dumper_is_xml(dumper)) {
            nx_dumper_emit(dumper, indent,
                    _LABEL("Flags")
                    "{dw:/%#x}{e:flags/%u}{d:/%s}\n",
                    nx_swap32(cib->cib_flags),
                    nx_swap32(cib->cib_flags),
                    _bitflag_names(nx_swap32(cib->cib_flags), NULL, NULL,
                        buf, sizeof(buf)));
            nx_dumper_emit(dumper, indent,
                    _LABEL("Count")
                    "{:count/%u}\n",
                    nx_swap32(cib->cib_count));
        }

        nx_dumper_emit(dumper, indent, _TITLE("Entries") "\n");

        {
            nx_dumper_table_t *table;

            _INC_INDENT();

            if (nx_dumper_is_text(dumper)) {
                table = nx_dumper_table_new(MAX_TABLE_WIDTH - indent,
                        "#",                "%" PRIuSIZE, NX_DUMPER_TABLE_CENTER,
                        "Checkpoint ID",    "%" PRIu64,   NX_DUMPER_TABLE_RIGHT,
                        "Physical Address", "%#" PRIx64,  NX_DUMPER_TABLE_RIGHT,
                        "Size",             "%#x",        NX_DUMPER_TABLE_RIGHT,
                        "?",                "%#x",        NX_DUMPER_TABLE_RIGHT,
                        "Chunk",            "%" PRIu64,   NX_DUMPER_TABLE_RIGHT,
                        NULL);
            } else if (nx_dumper_is_xml(dumper)) {
                nx_dumper_open_container(dumper, "entries");
            } else {
                nx_dumper_open_list(dumper, "entries");
            }

            for (n = 0; n < nx_swap32(cib->cib_count); n++) {
                if (nx_dumper_is_text(dumper)) {
                    nx_dumper_table_add(table, n,
                            nx_swap64(cib->cib_map[n].cib_xid),
                            nx_swap64(cib->cib_map[n].cib_paddr),
                            nx_swap32(cib->cib_map[n].cib_size),
                            nx_swap32(cib->cib_map[n].cib_unknown_x10),
                            nx_swap64(cib->cib_map[n].cib_chunk));
                } else {
                    if (nx_dumper_is_xml(dumper)) {
                        nx_dumper_attr(dumper, "index", "%" PRIuSIZE, n);

                        nx_dumper_open_container(dumper, "entry");
                    } else {
                        nx_dumper_open_instance(dumper);
                    }

                    nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                            "{e:xid/%" PRIu64 "}"
                            "{e:physical-address/%" PRIu64 "}"
                            "{e:size/%u}"
                            "{e:unknown-x10/%u}"
                            "{e:chunk/%" PRIu64 "}",
                            nx_swap64(cib->cib_map[n].cib_xid),
                            nx_swap64(cib->cib_map[n].cib_paddr),
                            nx_swap32(cib->cib_map[n].cib_size),
                            nx_swap32(cib->cib_map[n].cib_unknown_x10),
                            nx_swap64(cib->cib_map[n].cib_chunk));

                    nx_dumper_close(dumper);
                }
            }

            if (nx_dumper_is_text(dumper)) {
                nx_dumper_emit_table(dumper, indent, table);
                nx_dumper_table_dispose(table);
            } else {
                nx_dumper_close(dumper);
            }

            _DEC_INDENT();
        }

        _DEC_INDENT();
    }
    nx_dumper_close(dumper);

    nx_dumper_flush(dumper);
}

void
nx_cpm_dump(nx_dumper_t *dumper, nx_cpm_t const *cpm)
{
    size_t n, indent = 0;
    char   buf[128];

    if (dumper == NULL || cpm == NULL)
        return;

    nx_dumper_emit(dumper, indent, _TITLE("NX Checkpoint Map") "\n");

    _nx_object_dump0(dumper, NX_OBJECT(cpm));
    nx_dumper_attr(dumper, "flags", "%u", nx_swap32(cpm->cpm_flags));
    nx_dumper_attr(dumper, "count", "%u", nx_swap32(cpm->cpm_count));

    nx_dumper_open_container(dumper, "cpm");
    {
        _INC_INDENT();

        _nx_object_dump1(dumper, indent, NX_OBJECT(cpm));
        if (!nx_dumper_is_xml(dumper)) {
            nx_dumper_emit(dumper, indent,
                    _LABEL("Flags")
                    "{dw:/%#x}{e:flags/%u}{d:/%s}\n",
                    nx_swap32(cpm->cpm_flags),
                    nx_swap32(cpm->cpm_flags),
                    _bitflag_names(nx_swap32(cpm->cpm_flags), NULL, NULL,
                        buf, sizeof(buf)));
            nx_dumper_emit(dumper, indent,
                    _LABEL("Count")
                    "{:count/%u}\n",
                    nx_swap32(cpm->cpm_count));
        }

        nx_dumper_emit(dumper, indent, _TITLE("Entries") "\n");

        {
            char               flag[32];
            char               meaning[128];
            nx_dumper_table_t *table;

            _INC_INDENT();

            if (nx_dumper_is_text(dumper)) {
                table = nx_dumper_table_new(MAX_TABLE_WIDTH - indent,
                        "#",            "%" PRIuSIZE, NX_DUMPER_TABLE_CENTER,
                        "Type",         "%#x",        NX_DUMPER_TABLE_RIGHT,
                        "Subtype",      "%#x",        NX_DUMPER_TABLE_RIGHT,
                        "Meaning",      "%s %s",      NX_DUMPER_TABLE_LEFT,
                        "Size",         "%#" PRIx64,  NX_DUMPER_TABLE_RIGHT,
                        "FS Object ID", "%" PRIu64,   NX_DUMPER_TABLE_RIGHT,
                        "Object ID",    "%" PRIu64,   NX_DUMPER_TABLE_RIGHT,
                        "Block #",      "%" PRIu64,   NX_DUMPER_TABLE_RIGHT,
                        NULL);
            } else if (nx_dumper_is_xml(dumper)) {
                nx_dumper_open_container(dumper, "entries");
            } else {
                nx_dumper_open_list(dumper, "entries");
            }

            for (n = 0; n < nx_swap32(cpm->cpm_count); n++) {
                if (nx_dumper_is_text(dumper)) {
                    nx_dumper_table_add(table, n,
                            nx_swap32(cpm->cpm_map[n].cpm_type),
                            nx_swap32(cpm->cpm_map[n].cpm_subtype),
                            _objectref_name(nx_swap32(cpm->cpm_map[n].cpm_type)),
                            nx_object_type_name_r(
                                nx_swap32(cpm->cpm_map[n].cpm_type) & 0xfffffff,
                                nx_swap32(cpm->cpm_map[n].cpm_subtype),
                                meaning, sizeof(meaning)),
                            nx_swap64(cpm->cpm_map[n].cpm_size),
                            nx_swap64(cpm->cpm_map[n].cpm_fs_oid),
                            nx_swap64(cpm->cpm_map[n].cpm_oid),
                            nx_swap64(cpm->cpm_map[n].cpm_paddr));
                } else {
                    if (nx_dumper_is_xml(dumper)) {
                        nx_dumper_attr(dumper, "index", "%" PRIuSIZE, n);
                        nx_dumper_open_container(dumper, "entry");
                    } else {
                        nx_dumper_open_instance(dumper);
                    }

                    nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                            "{e:type/%u}"
                            "{e:sub-type/%u}"
                            "{e:size/%" PRIu64 "}"
                            "{e:file-system-oid/%" PRIu64 "}"
                            "{e:oid/%" PRIu64 "}"
                            "{e:physical-address/%" PRIu64 "}",
                            nx_swap32(cpm->cpm_map[n].cpm_type),
                            nx_swap32(cpm->cpm_map[n].cpm_subtype),
                            nx_swap64(cpm->cpm_map[n].cpm_size),
                            nx_swap64(cpm->cpm_map[n].cpm_fs_oid),
                            nx_swap64(cpm->cpm_map[n].cpm_oid),
                            nx_swap64(cpm->cpm_map[n].cpm_paddr));

                    nx_dumper_close(dumper);
                }
            }

            if (nx_dumper_is_text(dumper)) {
                nx_dumper_emit_table(dumper, indent, table);
                nx_dumper_table_dispose(table);
            } else {
                nx_dumper_close(dumper);
            }

            _DEC_INDENT();
        }

        _DEC_INDENT();
    }
    nx_dumper_close(dumper);

    nx_dumper_flush(dumper);
}

void
nx_spaceman_dump(nx_dumper_t *dumper, nx_spaceman_t const *sm)
{
    size_t          n, indent = 0;
    uint64_t const *xids;
    uint16_t const *bms;

    if (dumper == NULL || sm == NULL)
        return;

    nx_dumper_emit(dumper, indent, _TITLE("NX Space Manager") "\n");

    _nx_object_dump0(dumper, NX_OBJECT(sm));
    nx_dumper_open_container(dumper, "spaceman");
    {
        _INC_INDENT();

        _nx_object_dump1(dumper, indent, NX_OBJECT(sm));
        nx_dumper_emit(dumper, indent,
                _LABEL("Block Size")
                "{:block-size/%u}\n",
                nx_swap32(sm->sm_block_size));
        nx_dumper_emit(dumper, indent,
                _LABEL("Blocks per Chunk")
                "{:blocks-per-chunk/%u}\n",
                nx_swap32(sm->sm_blocks_per_chunk));
        nx_dumper_emit(dumper, indent,
                _LABEL("Chunks per CIB")
                "{:chunks-per-cib/%u}\n",
                nx_swap32(sm->sm_chunks_per_cib));
        nx_dumper_emit(dumper, indent,
                _LABEL("CIBs per CAB")
                "{:cibs-per-cab/%u}\n",
                nx_swap32(sm->sm_cibs_per_cab));
        nx_dumper_emit(dumper, indent,
                _LABEL("Block Count")
                "{w:block-count/%" PRIu64 "}"
                "({h,hn-1000,hn-decimal:bytes-capacity/%" PRIu64 "})\n",
                nx_swap64(sm->sm_block_count),
                nx_swap32(sm->sm_block_size) * nx_swap64(sm->sm_block_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("Chunk Count")
                "{:chunk-count/%" PRIu64 "}\n",
                nx_swap64(sm->sm_chunk_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("CIB Count")
                "{:cib-count/%u}\n",
                nx_swap64(sm->sm_cib_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("CAB Count")
                "{:cab-count/%u}\n",
                nx_swap64(sm->sm_cab_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("Free Block Count")
                "{w:free-block-count/%" PRIu64 "}"
                "({h,hn-1000,hn-decimal:bytes-capacity/%" PRIu64 "})\n",
                nx_swap64(sm->sm_free_count),
                nx_swap32(sm->sm_block_size) * nx_swap64(sm->sm_free_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("Free Queue Count")
                "{:free-queue-count/%" PRIu64 "}\n",
                nx_swap64(sm->sm_free_queue_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("Tier-2 Block Count")
                "{w:tier2-block-count/%" PRIu64 "}"
                "({h,hn-1000,hn-decimal:bytes-capacity/%" PRIu64 "})\n",
                nx_swap64(sm->sm_tier2_block_count),
                nx_swap32(sm->sm_block_size) *
                nx_swap64(sm->sm_tier2_block_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("Tier-2 Chunk Count")
                "{:tier2-chunk-count/%" PRIu64 "}\n",
                nx_swap64(sm->sm_tier2_chunk_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("Tier-2 CIB Count")
                "{:tier2-cib-count/%u}\n",
                nx_swap64(sm->sm_tier2_cib_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("Tier-2 CAB Count")
                "{:tier2-cab-count/%u}\n",
                nx_swap64(sm->sm_tier2_cab_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("Tier-2 Free Block Count")
                "{w:tier2-free-block-count/%" PRIu64 "}"
                "({h,hn-1000,hn-decimal:bytes-capacity/%" PRIu64 "})\n",
                nx_swap64(sm->sm_tier2_free_count),
                nx_swap32(sm->sm_block_size) *
                nx_swap64(sm->sm_tier2_free_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("Tier-2 Free Queue Count")
                "{:tier2-free-queue-count/%" PRIu64 "}\n",
                nx_swap64(sm->sm_tier2_free_queue_count));

        nx_dumper_emit(dumper, indent, _TITLE("Internal Pool") "\n");

        nx_dumper_open_container(dumper, "internal-pool");
        {
            _INC_INDENT();

            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("First Block")
                    "{w:first-block/%" PRIu64 "}"
                    "{d:/(%#" PRIx64 ")}\n",
                    nx_swap64(sm->sm_ip_block_first),
                    nx_swap64(sm->sm_ip_block_first));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Block Count")
                    "{:block-count/%u}\n",
                    nx_swap32(sm->sm_ip_block_count));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("First Bitmap Block")
                    "{w:first-bitmap-block/%" PRIu64 "}"
                    "{d:/(%#" PRIx64 ")}\n",
                    nx_swap64(sm->sm_ip_bitmap_block_first),
                    nx_swap64(sm->sm_ip_bitmap_block_first));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Bitmap Block Count")
                    "{:bitmap-block-count/%u}\n",
                    nx_swap32(sm->sm_ip_bitmap_block_count));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Block Map Block Count")
                    "{:bm-block-count/%u}\n",
                    nx_swap32(sm->sm_ip_bm_block_count));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Block Map Next Free Slot")
                    "{:bm-next-array-free/%u}\n",
                    nx_swap32(sm->sm_ip_bm_next_array_free));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Free Queue Count")
                    "{:free-queue-count/%" PRIu64 "}\n",
                    nx_swap64(sm->sm_ip_free_queue_count));

            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        nx_dumper_emit(dumper, indent,
                _LABEL("Tree Object ID")
                "{w:tree-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(sm->sm_tree_oid),
                nx_swap64(sm->sm_tree_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Checkpoints Offset")
                "{:xid-offset/%u} "
                "{d:/(%#x)}\n",
                nx_swap32(sm->sm_xid_offset),
                nx_swap32(sm->sm_xid_offset));
        nx_dumper_emit(dumper, indent,
                _LABEL("Block Map Offset")
                "{:bm-offset/%u} "
                "{d:/(%#x)}\n",
                nx_swap32(sm->sm_bm_offset),
                nx_swap32(sm->sm_bm_offset));

        xids = (uint64_t const *)((uintptr_t)sm + nx_swap32(sm->sm_xid_offset));
        bms = (uint16_t const *)((uintptr_t)sm + nx_swap32(sm->sm_bm_offset));

        nx_dumper_emit(dumper, indent, _TITLE("Block Map") "\n");
        {
            nx_dumper_table_t *table;

            _INC_INDENT();

            if (nx_dumper_is_text(dumper)) {
                table = nx_dumper_table_new(MAX_TABLE_WIDTH - indent,
                        "#",             "%" PRIuSIZE, NX_DUMPER_TABLE_CENTER,
                        "Checkpoint ID", "%" PRIu64,   NX_DUMPER_TABLE_RIGHT,
                        "Block #",       "%" PRIu64,   NX_DUMPER_TABLE_RIGHT,
                        NULL);
            } else if (nx_dumper_is_xml(dumper)) {
                nx_dumper_open_container(dumper, "block-map");
            } else {
                nx_dumper_open_list(dumper, "block-map");
            }

            for (n = 0; n < nx_swap32(sm->sm_ip_bm_block_count); n++) {
                if (nx_dumper_is_text(dumper)) {
                    nx_dumper_table_add(table, n,
                            nx_swap64(xids[n]), nx_swap16(bms[n]));
                } else {
                    if (nx_dumper_is_xml(dumper)) {
                        nx_dumper_attr(dumper, "index", "%" PRIuSIZE, n);
                        nx_dumper_open_container(dumper, "entry");
                    } else {
                        nx_dumper_open_instance(dumper);
                    }

                    nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                            "{e:xid/%" PRIu64 "}"
                            "{e:block/%u}",
                            nx_swap64(xids[n]),
                            nx_swap16(bms[n]));

                    nx_dumper_close(dumper);
                }
            }

            if (nx_dumper_is_text(dumper)) {
                nx_dumper_emit_table(dumper, indent, table);
                nx_dumper_table_dispose(table);
            } else {
                nx_dumper_close(dumper);
            }

            _DEC_INDENT();
        }

        _DEC_INDENT();
    }
    nx_dumper_close(dumper);

    nx_dumper_flush(dumper);
}

void
nx_omap_dump(nx_dumper_t *dumper, nx_omap_t const *om)
{
    char   buf[128];
    size_t indent = 0;

    if (dumper == NULL || om == NULL)
        return;

    nx_dumper_emit(dumper, indent, _TITLE("NX Object Map") "\n");

    nx_dumper_attr(dumper, "flags", "%u", nx_swap32(om->om_flags));

    _nx_object_dump0(dumper, NX_OBJECT(om));
    nx_dumper_open_container(dumper, "omap");
    {
        _INC_INDENT();

        _nx_object_dump1(dumper, indent, NX_OBJECT(om));
        if (!nx_dumper_is_xml(dumper)) {
            nx_dumper_emit(dumper, indent,
                    _LABEL("Flags")
                    "{dw:/%#x}{e:flags/%u}{d:/%s}\n",
                    nx_swap32(om->om_flags),
                    nx_swap32(om->om_flags),
                    _bitflag_names(nx_swap32(om->om_flags), NULL,
                        "\001SNAPSHOT", buf, sizeof(buf)));
        }
        nx_dumper_emit(dumper, indent,
                _LABEL("Tree Type")
                "{d:/%#x}{e:tree-type/%u}\n",
                nx_swap32(om->om_tree_type),
                nx_swap32(om->om_tree_type));
        nx_dumper_emit(dumper, indent,
                _LABEL("Tree Object ID")
                "{w:tree-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(om->om_tree_oid),
                nx_swap64(om->om_tree_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Snapshot Count")
                "{:snap-count/%u}\n",
                nx_swap32(om->om_snap_count));
        nx_dumper_emit(dumper, indent,
                _LABEL("Snapshot Tree Type")
                "{d:/%#x}{e:snap-tree-type/%u}\n",
                nx_swap32(om->om_snapshot_tree_type),
                nx_swap32(om->om_snapshot_tree_type));
        nx_dumper_emit(dumper, indent,
                _LABEL("Snapshot Tree Object ID")
                "{w:snap-tree-oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(om->om_snapshot_tree_oid),
                nx_swap64(om->om_snapshot_tree_oid));
        nx_dumper_emit(dumper, indent,
                _LABEL("Most Recent Snapshot")
                "{w:most-recent-snap/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(om->om_most_recent_snap),
                nx_swap64(om->om_most_recent_snap));
        nx_dumper_emit(dumper, indent,
                _LABEL("Pending Revert Minimum")
                "{w:pending-revert-min/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(om->om_pending_revert_min),
                nx_swap64(om->om_pending_revert_min));
        nx_dumper_emit(dumper, indent,
                _LABEL("Pending Revert Maximum")
                "{w:pending-revert-max/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(om->om_pending_revert_max),
                nx_swap64(om->om_pending_revert_max));

        _DEC_INDENT();
    }
    nx_dumper_close(dumper);

    nx_dumper_flush(dumper);
}

void
nx_btn_dump(nx_dumper_t *dumper, nx_btn_t const *btn, nx_btn_t const *btntop)
{
    size_t             n, indent = 0;
    char               buf[128];
    bt_fixed_t const  *bt;
    bool               root;
    bool               leaf;
    static char const *key_l0_format = _SUBLABEL2("Key (Data)") "{d:/%s}\n";
    static char const *key_l1_format = _SPACE2("") "{d:/%s}\n";
    static char const *val_l0_format = _SUBLABEL2("Value (Data)") "{d:/%s}\n";
    static char const *val_l1_format = _SPACE2("") "{d:/%s}\n";

    if (dumper == NULL || btn == NULL)
        return;

    root = (NX_OBJECT_GET_TYPE(nx_swap32(NX_OBJECT(btn)->o_type)) ==
            NX_OBJECT_TYPE_BTREE_ROOT);
    leaf = nx_swap16(btn->btn_level) == 0;

    nx_dumper_emit(dumper, indent, _TITLE("/NX B+Tree %s") "\n",
            root ? "Root" : "Node");

    _nx_object_dump0(dumper, NX_OBJECT(btn));
    nx_dumper_attr(dumper, "flags", "%u", nx_swap16(btn->btn_flags));
    nx_dumper_attr(dumper, "level", "%u", nx_swap16(btn->btn_level));
    nx_dumper_attr(dumper, "nkeys", "%u", nx_swap32(btn->btn_nkeys));
    nx_dumper_attr(dumper, "root", "%s", root ? "true" : "false");

    nx_dumper_open_container(dumper, "btree-node");
    {
        _INC_INDENT();

        _nx_object_dump1(dumper, indent, NX_OBJECT(btn));
        if (!nx_dumper_is_xml(dumper)) {
            nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                    "{e:root/%s}",
                    root ? "true" : "false");
            nx_dumper_emit(dumper, indent,
                    _LABEL("Flags")
                    "{dw:/%#x}{e:flags/%u}{d:/%s}\n",
                    nx_swap16(btn->btn_flags),
                    nx_swap16(btn->btn_flags),
                    _bitflag_names(nx_swap16(btn->btn_flags), NULL,
                        "\001FIXED\002LEAF\003COMPRESSED",
                        buf, sizeof(buf)));
            nx_dumper_emit(dumper, indent,
                    _LABEL("Level")
                    "{d:/%#x}{e:level/%u}\n",
                    nx_swap16(btn->btn_level),
                    nx_swap16(btn->btn_level));
            nx_dumper_emit(dumper, indent,
                    _LABEL("Key Count")
                    "{d:/%#x}{e:key-count/%u}\n",
                    nx_swap16(btn->btn_nkeys),
                    nx_swap16(btn->btn_nkeys));
        }

        nx_dumper_emit(dumper, indent, _TITLE("Table Space") "\n");
        nx_dumper_open_container(dumper, "table-space");
        {
            _INC_INDENT();

            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Offset")
                    "{w:offset/%u}"
                    "{d:/(%#x)}\n",
                    nx_swap16(btn->btn_table_space.offset),
                    nx_swap16(btn->btn_table_space.offset));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Length")
                    "{:length/%u}\n",
                    nx_swap16(btn->btn_table_space.len));

            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        nx_dumper_emit(dumper, indent, _TITLE("Free Space") "\n");
        nx_dumper_open_container(dumper, "free-space");
        {
            _INC_INDENT();

            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Offset")
                    "{w:offset/%u}"
                    "{d:/(%#x)}\n",
                    nx_swap16(btn->btn_free_space.offset),
                    nx_swap16(btn->btn_free_space.offset));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Length")
                    "{:length/%u}\n",
                    nx_swap16(btn->btn_free_space.len));

            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        nx_dumper_emit(dumper, indent, _TITLE("Key Free List") "\n");
        nx_dumper_open_container(dumper, "key-free-list");
        {
            _INC_INDENT();

            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Offset")
                    "{w:offset/%u}"
                    "{d:/(%#x)}\n",
                    nx_swap16(btn->btn_key_free_list.offset),
                    nx_swap16(btn->btn_key_free_list.offset));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Length")
                    "{:length/%u}\n",
                    nx_swap16(btn->btn_key_free_list.len));

            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        nx_dumper_emit(dumper, indent, _TITLE("Value Free List") "\n");
        nx_dumper_open_container(dumper, "value-free-list");
        {
            _INC_INDENT();

            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Offset")
                    "{w:offset/%u}"
                    "{d:/(%#x)}\n",
                    nx_swap16(btn->btn_val_free_list.offset),
                    nx_swap16(btn->btn_val_free_list.offset));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Length")
                    "{:length/%u}\n",
                    nx_swap16(btn->btn_val_free_list.len));

            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        /*
         * Extension nodes do refer the first node for the fixed part, dumping
         * makes no sense.
         */
        if (NX_OBJECT_GET_TYPE(nx_swap32(NX_OBJECT(btn)->o_type)) ==
                NX_OBJECT_TYPE_BTREE_NODE && btntop == NULL)
            goto done;

        if (btntop != NULL && nx_swap16(btn->btn_level) >=
                nx_swap16(btntop->btn_level))
            goto done;

        if (btntop == NULL) {
            btntop = btn;
        }

        nx_dumper_emit(dumper, indent, _TITLE("Root Trailer") "\n");

        bt = NX_BTN_FIXED(btntop);
        nx_dumper_attr(dumper, "flags", "%u", nx_swap32(bt->bt_flags));
        nx_dumper_open_container(dumper, "root-trailer");
        {
            _INC_INDENT();

            if (!nx_dumper_is_xml(dumper)) {
                nx_dumper_emit(dumper, indent,
                        _SUBLABEL("Flags")
                        "{dw:/%#x}{e:flags/%u}{d:/%s}\n",
                        nx_swap32(bt->bt_flags),
                        nx_swap32(bt->bt_flags),
                        _bitflag_names(nx_swap32(bt->bt_flags), NULL,
                            NULL, buf, sizeof(buf)));
            }

            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Node Size")
                    "{:node-size/%u}\n",
                    nx_swap32(bt->bt_node_size));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Key Size")
                    "{:key-size/%u}\n",
                    nx_swap32(bt->bt_key_size));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Value Size")
                    "{:value-size/%u}\n",
                    nx_swap32(bt->bt_val_size));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Longest Key Size")
                    "{:longest-key-size/%u}\n",
                    nx_swap32(bt->bt_longest_key));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Longest Value Size")
                    "{:longest-value-size/%u}\n",
                    nx_swap32(bt->bt_longest_val));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Key Count")
                    "{:key-count/%" PRIu64 "}\n",
                    nx_swap64(bt->bt_key_count));
            nx_dumper_emit(dumper, indent,
                    _SUBLABEL("Node Count")
                    "{:node-count/%" PRIu64 "}\n",
                    nx_swap64(bt->bt_node_count));

            _DEC_INDENT();
        }
        nx_dumper_close(dumper);

        if (btn->btn_nkeys == 0)
            goto done;

        nx_dumper_emit(dumper, indent, _TITLE("Entries") "\n");
        {
            nx_dumper_table_t *table;

            _INC_INDENT();

            if (nx_dumper_is_text(dumper)) {
                table = nx_dumper_table_new(MAX_TABLE_WIDTH - indent,
                        "#",             "%" PRIuSIZE, NX_DUMPER_TABLE_CENTER,
                        "Key Offset",    "%#x",        NX_DUMPER_TABLE_RIGHT,
                        "Key Size",      "%u",         NX_DUMPER_TABLE_RIGHT,
                        "Value Offset",  "%#x",        NX_DUMPER_TABLE_RIGHT,
                        "Value Size",    "%u",         NX_DUMPER_TABLE_RIGHT,
                        NULL);
            } else if (nx_dumper_is_xml(dumper)) {
                nx_dumper_open_container(dumper, "entries");
            } else {
                nx_dumper_open_list(dumper, "entries");
            }

            for (n = 0;; n++) {
                btn_kvinfo_t  kvi;
                void const   *key;
                void const   *val;

                if (!nx_btn_get_kvinfo(btn, btntop, n, &kvi))
                    break;
                if (!nx_btn_get_kvptrs(btn, btntop, &kvi, &key, &val))
                    break;

                if (nx_dumper_is_text(dumper)) {
                    nx_dumper_table_add(table, n,
                            kvi.key_offset, kvi.key_size,
                            kvi.val_offset, kvi.val_size);
                } else {
                    if (nx_dumper_is_xml(dumper)) {
                        nx_dumper_attr(dumper, "index", "%" PRIuSIZE, n);

                        nx_dumper_open_container(dumper, "entry");
                    } else {
                        nx_dumper_open_instance(dumper);
                    }

                    nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT,
                            "{e:key-offset/%u}"
                            "{e:key-size/%u}"
                            "{e:value-offset/%u}"
                            "{e:value-size/%u}",
                            kvi.key_offset, kvi.key_size,
                            kvi.val_offset, kvi.val_size);

                    nx_dumper_close(dumper);
                }
            }

            if (nx_dumper_is_text(dumper)) {
                nx_dumper_emit_table(dumper, indent, table);
                nx_dumper_table_dispose(table);
            } else {
                nx_dumper_close(dumper);
            }

            _DEC_INDENT();
        }

        for (n = 0; nx_dumper_is_text(dumper); n++) {
            btn_kvinfo_t  kvi;
            void const   *key;
            void const   *val;

            if (!nx_btn_get_kvinfo(btn, btntop, n, &kvi))
                break;
            if (!nx_btn_get_kvptrs(btn, btntop, &kvi, &key, &val))
                break;

            _INC_INDENT();

            nx_dumper_emit(dumper, indent, _TITLE("/Entry #%" PRIuSIZE) "\n",
                    n);

            if (key != NULL) {
                size_t m;

                _INC_INDENT();

                nx_dumper_emit(dumper, indent,
                        _SUBLABEL2("Key (Object ID)")
                        "{dw:/%" PRIu64 "}{d:/(%#" PRIx64 ")}\n",
                        nx_swap64(((uint64_t const *)key)[0]),
                        nx_swap64(((uint64_t const *)key)[0]),
                        "");

                if (leaf) {
                    char           buf[128];
                    bool           first    = true;
                    size_t         key_size = kvi.key_size;
                    uint8_t const *key_data = (uint8_t const *)key +
                        sizeof(uint64_t);

                    while (key_size != 0) {
                        size_t len = _hexdump(key_data, key_size,
                                buf, sizeof(buf));

                        nx_dumper_emit(dumper, indent,
                                       first ? key_l0_format : key_l1_format,
                                       buf);

                        key_data += len, key_size -= len, first = false;
                    }
                }

                _DEC_INDENT();
            }

            if (val != NULL) {
                size_t m;

                _INC_INDENT();

                if (leaf) {
                    char           buf[128];
                    bool           first    = true;
                    size_t         val_size = kvi.val_size;
                    uint8_t const *val_data = (uint8_t const *)val;

                    while (val_size != 0) {
                        size_t len = _hexdump(val_data, val_size,
                                buf, sizeof(buf));

                        nx_dumper_emit(dumper, indent,
                                first ? val_l0_format : val_l1_format,
                                buf);

                        val_data += len, val_size -= len;
                    }
                } else {
                    nx_dumper_emit(dumper, indent,
                            _SUBLABEL2("Value (Block #)")
                            "{dw:/%" PRIu64 "}"
                            "{d:/(%#" PRIx64 ")}"
                            "\n",
                            nx_swap64(((uint64_t const *)val)[0]),
                            nx_swap64(((uint64_t const *)val)[0]));
                }

                _DEC_INDENT();
            }

            _DEC_INDENT();
        }

        _DEC_INDENT();
    }

done:
    nx_dumper_close(dumper);

    nx_dumper_flush(dumper);
}

static void
nx_unknown_dump(nx_dumper_t *dumper, nx_object_t const *object)
{
    nx_dumper_emit(dumper, 0,
            _TITLE("/NX Unknown %s Object Type %#x.%u") "\n",
            _objectref_name(nx_swap32(object->o_type)),
            NX_OBJECT_GET_TYPE(nx_swap32(object->o_type)),
            nx_swap32(object->o_subtype));

    _nx_object_dump0(dumper, object);
    nx_dumper_open_container(dumper, "unknown-object");
    {
        size_t         size = NX_OBJECT_SIZE - sizeof(nx_object_t);
        uint8_t const *data = (uint8_t const *)(object + 1);

        _nx_object_dump1(dumper, 4, object);

        if (nx_dumper_is_text(dumper)) {
            static char const *l0_format =
                _SUBLABEL("Contents") "{d:/%s}\n";
            static char const *l1_format =
                _SPACE1("") "{d:/%s}\n";
            char buf[128];
            bool first = true;

            while (size != 0) {
                size_t len = _hexdump(data, size, buf, sizeof(buf));

                nx_dumper_emit(dumper, 4,
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

    nx_dumper_flush(dumper);
}

void
nx_object_dump(nx_dumper_t *dumper, nx_object_t const *object)
{
    if (dumper == NULL || object == NULL)
        return;

#define CALL_DUMPER(name) \
    nx_##name##_dump(dumper, (nx_##name##_t const *)object)

    switch (NX_OBJECT_GET_TYPE(nx_swap32(object->o_type))) {
        case NX_OBJECT_TYPE_CONTAINER:
            CALL_DUMPER(super);
            break;

        case NX_OBJECT_TYPE_CIB:
            CALL_DUMPER(cib);
            break;

        case NX_OBJECT_TYPE_CHECKPOINT_MAP:
            CALL_DUMPER(cpm);
            break;

        case NX_OBJECT_TYPE_SPACEMAN:
            CALL_DUMPER(spaceman);
            break;

        case NX_OBJECT_TYPE_OBJECT_MAP:
            CALL_DUMPER(omap);
            break;

        case NX_OBJECT_TYPE_BTREE_ROOT:
        case NX_OBJECT_TYPE_BTREE_NODE:
            nx_btn_dump(dumper, (nx_btn_t const *)object, NULL);
            break;

        case NX_OBJECT_TYPE_APFS_VOLUME:
            apfs_fs_dump(dumper, (apfs_fs_t const *)object);
            break;

        default:
            nx_unknown_dump(dumper, object);
            break;
    }
}
