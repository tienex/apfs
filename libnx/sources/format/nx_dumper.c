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

#include "nx/format/nx_dumper.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define NO_PRINTFLIKE
#ifdef HAVE_LIBXO_XO_H
#include <libxo/xo.h>
#else
#include "xo.h"
#endif
#include "table.h"

#ifndef SIZE_T_MAX
#define SIZE_T_MAX ((size_t)(-1))
#endif

typedef struct _nx_dumper_container {
    struct _nx_dumper_container *next;
    char                        *instance_name;
    unsigned                     type;
} nx_dumper_container_t;

struct _nx_dumper {
    FILE                  *fp;
    xo_handle_t           *xo;
    nx_dumper_container_t *stack;
    size_t                 indent;
};

struct _nx_dumper_table {
    size_t       maxwidth;
    struct table table;
};

nx_dumper_t *
nx_dumper_new(FILE *fp, unsigned style, unsigned flags)
{
    nx_dumper_t    *dumper;
    xo_style_t      xo_style;
    xo_xof_flags_t  xo_flags = XOF_NO_ENV | XOF_DTRT | XOF_UNDERSCORES |
     XOF_NO_TOP;

    if (fp == NULL)
        return NULL;

    switch (style) {
        case NX_DUMPER_STYLE_TEXT:
            xo_style = XO_STYLE_TEXT;
            break;
        case NX_DUMPER_STYLE_JSON:
            xo_style = XO_STYLE_JSON;
            break;
        case NX_DUMPER_STYLE_XML:
            xo_style = XO_STYLE_XML;
            break;
        default:
            return NULL;
    }

    if (flags & NX_DUMPER_FLAG_PRETTY) {
        xo_flags |= XOF_PRETTY;
    }

    dumper = (nx_dumper_t *)calloc(1, sizeof(nx_dumper_t));
    if (dumper == NULL)
        return NULL;

    dumper->xo = xo_create_to_file(fp, xo_style, xo_flags);
    if (dumper->xo == NULL) {
        free(dumper);
        return NULL;
    }

    dumper->fp = fp;

    return dumper;
}

void
nx_dumper_dispose(nx_dumper_t *dumper)
{
    nx_dumper_container_t *c, *next;

    if (dumper == NULL)
        return;

    if (dumper->xo != NULL) {
        while (dumper->stack != NULL) {
            nx_dumper_close(dumper);
        }

        xo_flush_h(dumper->xo);

        fflush(dumper->fp);

        xo_destroy(dumper->xo);
    }

    free(dumper);
}

size_t
nx_dumper_get_indent(nx_dumper_t const *dumper)
{
    return (dumper != NULL) ? dumper->indent : 0;
}

bool
nx_dumper_set_indent(nx_dumper_t *dumper, size_t indent)
{
    if (dumper == NULL || indent == NX_DUMPER_NO_INDENT)
        return false;

    dumper->indent = indent;
    return true;
}

unsigned
nx_dumper_get_style(nx_dumper_t const *dumper)
{
    if (dumper != NULL) {
        switch (xo_get_style(dumper->xo)) {
            case XO_STYLE_TEXT: return NX_DUMPER_STYLE_TEXT;
            case XO_STYLE_JSON: return NX_DUMPER_STYLE_JSON;
            case XO_STYLE_XML:  return NX_DUMPER_STYLE_XML;
        }
    }

    return NX_DUMPER_STYLE_INVALID;
}

#define nx_dumper_is_text(dumper) \
    (nx_dumper_get_style(dumper) == NX_DUMPER_STYLE_TEXT)
#define nx_dumper_is_json(dumper) \
    (nx_dumper_get_style(dumper) == NX_DUMPER_STYLE_JSON)
#define nx_dumper_is_xml(dumper) \
    (nx_dumper_get_style(dumper) == NX_DUMPER_STYLE_XML)

bool nx_dumper_emit(nx_dumper_t *dumper, size_t indent, char const *format, ...);

bool
nx_dumper_vemit(nx_dumper_t *dumper, size_t indent, char const *format,
        va_list ap)
{
    if (dumper == NULL || format == NULL)
        return false;

    if (*format == '\0')
        return true;

    if (indent != NX_DUMPER_NO_INDENT) {
        if (!nx_dumper_emit(dumper, NX_DUMPER_NO_INDENT, "{d:/%*s}",
                    (int)(dumper->indent + indent), ""))
            return false;
    }

    return !(xo_emit_hv(dumper->xo, format, ap) < 0);
}

bool
nx_dumper_emit(nx_dumper_t *dumper, size_t indent, char const *format, ...)
{
    va_list ap;
    bool    success;

    if (dumper == NULL || format == NULL)
        return false;

    if (*format == '\0')
        return true;

    va_start(ap, format);
    success = nx_dumper_vemit(dumper, indent, format, ap);
    va_end(ap);

    return success;
}

bool
nx_dumper_vattr(nx_dumper_t *dumper, char const *name, char const *format,
        va_list ap)
{
    if (dumper == NULL || format == NULL)
        return false;

    if (*format == '\0')
        return true;

    return !(xo_attr_hv(dumper->xo, name, format, ap) < 0);
}

bool
nx_dumper_attr(nx_dumper_t *dumper, char const *name, char const *format, ...)
{
    va_list ap;
    bool    success;

    if (dumper == NULL || format == NULL)
        return false;

    if (*format == '\0')
        return true;

    va_start(ap, format);
    success = nx_dumper_vattr(dumper, name, format, ap);
    va_end(ap);

    return success;
}

bool
nx_dumper_open_container(nx_dumper_t *dumper, char const *name)
{
    nx_dumper_container_t *c;

    if (dumper == NULL || name == NULL || *name == '\0')
        return false;

    c = (nx_dumper_container_t *)calloc(1, sizeof(nx_dumper_container_t));
    if (c == NULL)
        return false;

    if (xo_open_container_hd(dumper->xo, name) < 0) {
        free(c);
        return false;
    }

    c->type = NX_DUMPER_CONTAINER_CONTAINER;
    c->next = dumper->stack;
    dumper->stack = c;

    return true;
}

bool
nx_dumper_open_list(nx_dumper_t *dumper, char const *name)
{
    nx_dumper_container_t *c;

    if (dumper == NULL || name == NULL || *name == '\0')
        return false;

    if (nx_dumper_is_xml(dumper))
        return nx_dumper_open_container(dumper, name);

    c = (nx_dumper_container_t *)calloc(1, sizeof(nx_dumper_container_t) +
            strlen(name) + 1);
    if (c == NULL)
        return false;

    if (xo_open_list_hd(dumper->xo, name) < 0) {
        free(c);
        return false;
    }

    c->type = NX_DUMPER_CONTAINER_LIST;
    c->instance_name = (char *)(c + 1);
    strcpy(c->instance_name, name);
    c->next = dumper->stack;
    dumper->stack = c;

    return true;
}

bool
nx_dumper_open_instance(nx_dumper_t *dumper)
{
    nx_dumper_container_t *c, *top;

    if (dumper == NULL)
        return false;

    c = (nx_dumper_container_t *)calloc(1, sizeof(nx_dumper_container_t));
    if (c == NULL)
        return false;

    top = dumper->stack;
    if (top == NULL || top->type != NX_DUMPER_CONTAINER_LIST) {
        c->type = NX_DUMPER_CONTAINER_DUMMY;
    } else {
        if (xo_open_instance_hd(dumper->xo, top->instance_name) < 0) {
            free(c);
            return false;
        }

        c->type = NX_DUMPER_CONTAINER_INSTANCE;
    }

    c->next = dumper->stack;
    dumper->stack = c;

    return true;
}

bool
nx_dumper_close(nx_dumper_t *dumper)
{
    nx_dumper_container_t *c;
    bool                   success;

    if (dumper == NULL || dumper->stack == NULL)
        return false;

    c = dumper->stack;
    dumper->stack = c->next;

    switch (c->type) {
        case NX_DUMPER_CONTAINER_DUMMY:
            success = true;
            break;
        case NX_DUMPER_CONTAINER_CONTAINER:
            success = !(xo_close_container_hd(dumper->xo) < 0);
            break;
        case NX_DUMPER_CONTAINER_LIST:
            success = !(xo_close_list_hd(dumper->xo) < 0);
            break;
        case NX_DUMPER_CONTAINER_INSTANCE:
            success = !(xo_close_instance_hd(dumper->xo) < 0);
            break;
    }

    free(c);

    return success;
}

bool
nx_dumper_flush(nx_dumper_t *dumper)
{
    if (dumper == NULL)
        return false;

    return !(xo_flush_h(dumper->xo) < 0);
}

nx_dumper_table_t *
nx_dumper_table_vnew(size_t maxwidth, va_list ap)
{
    nx_dumper_table_t *table;

    if (maxwidth == 0) {
        maxwidth = SIZE_T_MAX;
    }

    table = (nx_dumper_table_t *)calloc(1, sizeof(nx_dumper_table_t));
    if (table != NULL) {
        table->maxwidth = maxwidth;
        if (!table_vinit(&table->table, ap)) {
            free(table);
            table = NULL;
        }
    }

    return table;
}

nx_dumper_table_t *
nx_dumper_table_new(size_t maxwidth, ...)
{
    va_list            ap;
    nx_dumper_table_t *table;

    va_start(ap, maxwidth);
    table = nx_dumper_table_vnew(maxwidth, ap);
    va_end(ap);

    return table;
}

void
nx_dumper_table_dispose(nx_dumper_table_t *table)
{
    if (table == NULL)
        return;

    table_free(&table->table);
    free(table);
}

bool
nx_dumper_table_vadd(nx_dumper_table_t *table, va_list ap)
{
    if (table == NULL)
        return false;

    return table_vadd(&table->table, ap);
}

bool
nx_dumper_table_add(nx_dumper_table_t *table, ...)
{
    va_list ap;
    bool    success;

    if (table == NULL)
        return false;

    va_start(ap, table);
    success = nx_dumper_table_vadd(table, ap);
    va_end(ap);

    return success;
}

bool
nx_dumper_emit_table(nx_dumper_t *dumper, size_t indent,
        nx_dumper_table_t const *table)
{
    if (dumper == NULL || table == NULL)
        return false;

    if (!nx_dumper_flush(dumper))
        return false;

    if (!nx_dumper_is_text(dumper))
        return true;

    if (indent != NX_DUMPER_NO_INDENT) {
        indent += dumper->indent;
    } else {
        indent = 0;
    }

    return table_print(&table->table, TABLE_OPT_NOSEP, indent,
            table->maxwidth, dumper->fp);
}
