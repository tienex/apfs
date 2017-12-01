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

#define _INC_INDENT() indent += 4
#define _DEC_INDENT() indent -= 4

#define MAX_TABLE_WIDTH 120

#define _TITLE(name)             "{T:" name "}"
#define _LABEL(name)             "{[:-40}{Lc:" name "}{]:}"
#define _SUBLABEL(name)          "{[:-36}{Lc:" name "}{]:}"
#define _SPACE1(name)            "{[:-36}{L:" name "}{]:}"
#define _SUBLABELx(before, name) "{[:-36}" before "{Lc:" name "}{]:}"
#define _SUBLABEL2(name)         "{[:-32}{Lc:" name "}{]:}"
#define _SPACE2(name)            "{[:-32}{L:" name "}{]:}"
#define _SUBLABEL3(name)         "{[:-28}{Lc:" name "}{]:}"
#define _SPACE3(name)            "{[:-28}{L:" name "}{]:}"
#define _SUBLABEL4(name)         "{[:-24}{Lc:" name "}{]:}"
#define _SPACE4(name)            "{[:-24}{L:" name "}{]:}"
#define _SUBLABEL5(name)         "{[:-20}{Lc:" name "}{]:}"

static size_t
_hexdump(uint8_t const *bytes, size_t count, char *buf, size_t bufsiz)
{
    // 16*3+1 + 1 + 16 + 1 + 1
    size_t n;

    if (count == 0)
        return 0;

    for (n = 0; n < count && n < 16; n++) {
        size_t c;

        if (n == 8 && bufsiz > 1) {
            *buf++ = ' ', bufsiz -= 1;
        }

        c = snprintf(buf, bufsiz, "%02X", bytes[n]);
        buf += c, bufsiz -= c;

        if (bufsiz > 1) {
            *buf++ = ' ', bufsiz -= 1;
        }
    }

    for (; n < 16; n++) {
        if (n == 8 && bufsiz > 1) {
            *buf++ = ' ', bufsiz -= 1;
        }

        if (bufsiz > 3) {
            *buf++ = ' ';
            *buf++ = ' ';
            *buf++ = ' ';
            bufsiz -= 3;
        }
    }

    if (bufsiz > 1) {
        *buf++ = ' ', bufsiz -= 1;
    }

    for (n = 0; n < count && n < 16; n++) {
        if (bufsiz > 1) {
            if (bytes[n] >= 0x20 && bytes[n] <= 0x7e) {
                *buf++ = bytes[n];
            } else {
                *buf++ = '.';
            }
            bufsiz--;
        }
    }

    *buf++ = '\0';

    return n;
}

static char const *
_bitflag_names(uint64_t value, char const *high, char const *low,
        char *buf, size_t bufsiz)
{
    size_t  n, m;
    char   *p = buf;

    if (buf == NULL || bufsiz == 0)
        return NULL;

    if (bufsiz > 1) {
        *p++ = '<', bufsiz--;
    }

    for (n = m = 0; bufsiz > 1 && n < 64; n++) {
        if (value & ((uint64_t)1 << n)) {
            size_t np;
            char const *name, *end;
            char const *names = (n > 31) ? high : low;
            if (names == NULL) {
                name = NULL;
            } else if (n > 31) {
                name = strchr(names, (char)(n - 31) + 1);
            } else {
                name = strchr(names, (char)(n + 1));
            }

            if (m++ != 0) {
                *p++ = ',', bufsiz--;
            }

            if (name == NULL) {
                np = snprintf(p, bufsiz, "b%" PRIuSIZE, n);
            } else {
                end = ++name;
                while (*end > 32) {
                    end++;
                }
                np = snprintf(p, bufsiz, "%.*s", (int)(end - name), name);
            }
            p += np, bufsiz -= np;
        }
    }

    if (bufsiz > 1) {
        *p++ = '>', bufsiz--;
    }

    *p = '\0';

    return buf;
}


static inline void
_nx_object_dump0(nx_dumper_t *dumper, nx_object_t const *o)
{
    if (nx_dumper_is_xml(dumper)) {
        nx_dumper_attr(dumper, "checksum", "%" PRIu64, nx_swap64(o->o_checksum));
        nx_dumper_attr(dumper, "oid", "%" PRIu64, nx_swap64(o->o_oid));
        nx_dumper_attr(dumper, "xid", "%" PRIu64, nx_swap64(o->o_xid));
        nx_dumper_attr(dumper, "type", "%u", nx_swap32(o->o_type));
        nx_dumper_attr(dumper, "subtype", "%u", nx_swap32(o->o_subtype));
    }
}

static inline void
_nx_object_dump1(nx_dumper_t *dumper, size_t indent, nx_object_t const *o)
{
    char buf[128];

    if (!nx_dumper_is_xml(dumper)) {
        nx_dumper_emit(dumper, indent, _TITLE("Object Header") "\n");

        nx_dumper_open_container(dumper, "object-header");

        indent += 4;

        nx_dumper_emit(dumper, indent,
                _SUBLABEL("Checksum")
                "{d:/%#" PRIx64 "}{e:checksum/%" PRIu64 "}\n",
                nx_swap64(o->o_checksum),
                nx_swap64(o->o_checksum));
        nx_dumper_emit(dumper, indent,
                _SUBLABEL("Object ID")
                "{w:oid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(o->o_oid),
                nx_swap64(o->o_oid));
        nx_dumper_emit(dumper, indent,
                _SUBLABEL("Checkpoint ID")
                "{w:xid/%" PRIu64 "}"
                "{d:/(%#" PRIx64 ")}\n",
                nx_swap64(o->o_xid),
                nx_swap64(o->o_xid));
        nx_dumper_emit(dumper, indent,
                _SUBLABEL("Type")
                "{w:type/%u}"
                "{d:/(%#x) %s}\n",
                NX_OBJECT_GET_TYPE(nx_swap32(o->o_type)),
                NX_OBJECT_GET_TYPE(nx_swap32(o->o_type)),
                _bitflag_names(nx_swap32(o->o_type) & 0xc0000000, NULL,
                    "\037DIRECT\040CHECKPOINT", buf, sizeof(buf)));
        nx_dumper_emit(dumper, indent,
                _SUBLABEL("Subtype")
                "{w:type/%u}"
                "{d:/(%#x)}\n",
                nx_swap32(o->o_subtype),
                nx_swap32(o->o_subtype));

        nx_dumper_close(dumper);
    }
}
