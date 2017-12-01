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

#ifndef __nx_format_nx_dumper_h
#define __nx_format_nx_dumper_h

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct _nx_dumper nx_dumper_t;
typedef struct _nx_dumper_table nx_dumper_table_t;

#define NX_DUMPER_NO_INDENT     ((size_t)(-1))

#define NX_DUMPER_STYLE_INVALID (~0U)
#define NX_DUMPER_STYLE_TEXT    0
#define NX_DUMPER_STYLE_JSON    1
#define NX_DUMPER_STYLE_XML     2

#define NX_DUMPER_FLAG_NONE     0
#define NX_DUMPER_FLAG_PRETTY   1

#define NX_DUMPER_CONTAINER_DUMMY     0
#define NX_DUMPER_CONTAINER_CONTAINER 1
#define NX_DUMPER_CONTAINER_LIST      2
#define NX_DUMPER_CONTAINER_INSTANCE  3

#define NX_DUMPER_TABLE_LEFT   0
#define NX_DUMPER_TABLE_RIGHT  1
#define NX_DUMPER_TABLE_CENTER 2

#ifdef __cplusplus
extern "C" {
#endif

nx_dumper_t *nx_dumper_new(FILE *fp, unsigned style, unsigned flags);
void nx_dumper_dispose(nx_dumper_t *dumper);

unsigned nx_dumper_get_style(nx_dumper_t const *dumper);

size_t nx_dumper_get_indent(nx_dumper_t const *dumper);
bool nx_dumper_set_indent(nx_dumper_t *dumper, size_t indent);

#define nx_dumper_is_text(dumper) \
    (nx_dumper_get_style(dumper) == NX_DUMPER_STYLE_TEXT)
#define nx_dumper_is_json(dumper) \
    (nx_dumper_get_style(dumper) == NX_DUMPER_STYLE_JSON)
#define nx_dumper_is_xml(dumper) \
    (nx_dumper_get_style(dumper) == NX_DUMPER_STYLE_XML)

bool nx_dumper_vemit(nx_dumper_t *dumper, size_t indent, char const *format,
        va_list ap);
bool nx_dumper_emit(nx_dumper_t *dumper, size_t indent, char const *format,
        ...);

bool nx_dumper_vattr(nx_dumper_t *dumper, char const *name, char const *format,
        va_list ap);
bool nx_dumper_attr(nx_dumper_t *dumper, char const *name, char const *format,
        ...);

bool nx_dumper_open_container(nx_dumper_t *dumper, char const *name);
bool nx_dumper_open_list(nx_dumper_t *dumper, char const *name);
bool nx_dumper_open_instance(nx_dumper_t *dumper);
bool nx_dumper_close(nx_dumper_t *dumper);

bool nx_dumper_flush(nx_dumper_t *dumper);

nx_dumper_table_t *nx_dumper_table_vnew(size_t maxwidth, va_list ap);
nx_dumper_table_t *nx_dumper_table_new(size_t maxwidth, ...);
void nx_dumper_table_dispose(nx_dumper_table_t *table);

bool nx_dumper_table_vadd(nx_dumper_table_t *table, va_list ap);
bool nx_dumper_table_add(nx_dumper_table_t *table, ...);

bool nx_dumper_emit_table(nx_dumper_t *dumper, size_t indent,
        nx_dumper_table_t const *table);

#ifdef __cplusplus
}
#endif

#endif  /* !__nx_format_nx_dumper_h */
