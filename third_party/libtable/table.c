#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <assert.h>

#include "table.h"

#define CPSTART(c) (((unsigned char) (c)) >> (CHAR_BIT - 2) != 2)


static char const BEGIN_DELIM = '\x1e';
static char const END_DELIM = '\x1f';
static char const *TOK_DELIM = "\x1f";
static char const CORNER = '+';
static char const INTERSECT = '*';

static inline size_t max(size_t a, size_t b)
{
        return a > b ? a : b;
}

static size_t utf8len(char const *s)
{
        size_t len = 0;
        char const *c;

        for (c = s; *c; ++c) {
                if (CPSTART(*c)) {
                        len += 1;
                }
        }

        return len;
}

static size_t b2cp(char const *s, size_t n)
{
        size_t cp, i;

        for (cp = i = 0; s[i] && n > 0; ++i, --n) {
                if (CPSTART(s[i])) {
                        cp += 1;
                }
        }

        return cp;
}

static void fputnc(int c, size_t n, FILE *f)
{
        while (n --> 0) {
                fputc(c, f);
        }
}

static size_t find_break(char const *s, size_t max, bool *hyphen)
{
        char const *c;
        size_t brk, cp;

        if (!*s) {
                *hyphen = false;
                return 0;
        }

        for (c = s, cp = brk = 0; *c && cp < max; ++c) {
                brk += 1;
                if (CPSTART(*c)) {
                        if (*c == '\n') {
                                break;
                        }
                        cp += 1;
                }
        }

        while (!CPSTART(*c)) {
                c += 1;
                brk += 1;
        }

        while (*c && c != s && !isspace(*c)) {
                c -= 1;
        }

        if (c == s) {
                *hyphen = true;
                while (!CPSTART(s[--brk]));
                return brk;
        }

        *hyphen = false;

        return c - s;
}

static inline void print_indent(size_t indent, FILE *f)
{
    fputnc(' ', indent, f);
}

static void print_row(size_t indent, char * const *data, size_t const *max,
        char const *align, size_t *remaining, size_t cols, FILE *f)
{
        size_t alloc = 0;
        char const **cells = NULL;

        size_t i, n, pad;
        bool hyphen, finished;

        cells = calloc(cols, sizeof(*cells));
        if (cells == NULL)
                return;

        for (i = 0; i < cols; ++i) {
                cells[i] = data[i];
                remaining[i] = utf8len(cells[i]);
        }

        for (finished = false; !finished;) {
                finished = true;
                print_indent(indent, f);
                for (i = 0; i < cols; ++i) {
                        fputs("| ", f);

                        n = find_break(cells[i], max[i], &hyphen);

                        pad = max[i] - b2cp(cells[i], n);
                        switch (align[i]) {
                                case TABLE_RIGHT:
                                        if (hyphen && pad > 0) {
                                            pad--;
                                            fputnc(' ', pad, f);
                                            pad = 1;
                                        } else {
                                            fputnc(' ', pad, f);
                                            pad = 0;
                                        }
                                        break;
                                case TABLE_CENTER:
                                        fputnc(' ', pad >> 1, f);
                                        pad -= pad >> 1;
                                        break;
                                default:
                                        break;
                        }

                        fwrite(cells[i], 1, n, f);

                        if (hyphen) {
                                fputc('-', f);
                        } else if (isspace(cells[i][n])) {
                                fputc(' ', f);
                                remaining[i] -= 1;
                                cells[i] += 1;
                        } else {
                                fputc(' ', f);
                        }

                        remaining[i] -= b2cp(cells[i], n);

                        if (remaining[i] != 0) {
                                finished = false;
                        }

                        fputnc(' ', pad, f);

                        cells[i] += n;

                        if (i + 1 == cols) {
                                fputs("|\n", f);
                        }
                }
        }

        free(cells);
}

bool table_vinit(struct table *t, va_list ap)
{
        int align;
        char const *header;
        char const *fmt;
        char *tmp, *tmpa, **tmph;
        size_t i, fmtlen, totalfmtlen;

        totalfmtlen = 0;

        memset(t, 0, sizeof(struct table));

        for (;; ++t->cols) {
                header = va_arg(ap, char *);
                if (header == NULL)
                        break;

                fmt = va_arg(ap, char *);
                if (fmt == NULL)
                        break;

                align = va_arg(ap, int);
                if (align < 0 || align > 2) {
                        align = 2;
                }

                tmph = realloc(t->headers, (t->cols + 1) * sizeof(char *));
                if (tmph == NULL)
                        goto fail;

                t->headers = tmph;

                t->headers[t->cols] = (char *)header;

                fmtlen = strlen(fmt);
                tmp = realloc(t->fmt, totalfmtlen + fmtlen + 3);
                if (tmp == NULL)
                        goto fail;

                t->fmt = tmp;
                t->fmt[totalfmtlen++] = BEGIN_DELIM;
                strcpy(t->fmt + totalfmtlen, fmt);
                totalfmtlen += fmtlen;
                t->fmt[totalfmtlen++] = END_DELIM;
                t->fmt[totalfmtlen] = '\0';

                tmpa = realloc(t->align, t->cols + 1);
                if (tmpa == NULL)
                        goto fail;

                t->align = tmpa;

                t->align[t->cols] = align;
        }

        t->max = calloc(t->cols, sizeof(*t->max));
        if (t->max == NULL) {
                free(t->headers);
                free(t->fmt);
                return false;
        }

        for (i = 0; i < t->cols; ++i) {
                t->max[i] = utf8len(t->headers[i]);
        }

        return true;

fail:
        table_free(t);
        return false;
}

bool table_init(struct table *t, ...)
{
        va_list ap;
        bool success;

        va_start(ap, t);
        success = table_vinit(t, ap);
        va_end(ap);

        return success;
}

bool table_vadd(struct table *t, va_list ap)
{
        char *field, **row, buffer[512];
        size_t i;

        if (t->rows == t->alloc) {
                char   ***tmp;
                size_t    capacity;

                capacity = t->alloc * 2 + 4;
                tmp      = realloc(t->data, capacity * sizeof(char **));
                if (tmp == NULL) {
                        return false;
                }
                memset(tmp + t->alloc, 0, (capacity - t->alloc) *
                        sizeof(char **));
                t->alloc = capacity;
                t->data = tmp;
        }

        row = (t->data[t->rows++] = calloc(t->cols, sizeof(char *)));

        if (row == NULL) {
                t->rows -= 1;
                return false;
        }

        vsnprintf(buffer, sizeof(buffer), t->fmt, ap);

        field = strtok(buffer, TOK_DELIM);

        for (i = 0; field != NULL; ++i, field = strtok(NULL, TOK_DELIM)) {
                field++; /* Skip front char */
                assert(&row[i] == &t->data[t->rows - 1][i]);
                row[i] = malloc(strlen(field) + 1);
                if (row[i] == NULL) {
                        goto err;
                }
                strcpy(row[i], field);

                t->max[i] = max(t->max[i], utf8len(field));
        }

        return true;

err:
        while (i --> 0) {
                free(row[i]);
        }

        return false;
}

bool table_add(struct table *t, ...)
{
        va_list ap;
        bool success;

        va_start(ap, t);
        success = table_vadd(t, ap);
        va_end(ap);

        return success;
}

bool table_print(struct table const *t, unsigned options, size_t indent,
        size_t n, FILE *f)
{
        size_t i, width, avg, trimthrshld, *max, *remaining;

        if (n < t->cols * 3 + 4) {
                /* Not enough space */
                return false;
        }

        n -= 2;

        max = calloc(t->cols, sizeof(*max));
        if (max == NULL) {
                return false;
        }

        remaining = calloc(t->cols, sizeof(*remaining));
        if (remaining == NULL) {
                free(max);
                return false;
        }

        width = t->cols * 3 + 1;
        for (i = 0; i < t->cols; ++i) {
                max[i] = t->max[i];
                width += t->max[i];
        }

        avg = n / t->cols;
        trimthrshld = 0;

        while (width > n) {
                bool none = true;
                for (i = 0; i < t->cols; ++i) {
                        if (max[i] + trimthrshld > avg) {
                                max[i] -= 1;
                                width -= 1;
                                none = false;
                        }
                }

                if (none) {
                        trimthrshld += 1;
                }
        }

        print_indent(indent, f);
        fputc(CORNER, f);
        fputnc('-', width - 2, f);
        fputc(CORNER, f);
        fputc('\n', f);

        print_row(indent, t->headers, max, t->align, remaining, t->cols, f);

        print_indent(indent, f);
        fputc(INTERSECT, f);
        fputnc('-', width - 2, f);
        fputc(INTERSECT, f);
        fputc('\n', f);

        for (i = 0; i < t->rows; ++i) {
                print_row(indent, t->data[i], max, t->align, remaining,
                        t->cols, f);

                if ((options & TABLE_OPT_NOSEP) == 0 && i + 1 < t->rows) {
                        size_t j;
                        print_indent(indent, f);
                        fputc('|', f);
                        for (j = 0; j < t->cols; ++j) {
                                fputnc('-', max[j] + 2, f);
                                fputc('|', f);
                        }
                        fputc('\n', f);
                }
        }

        print_indent(indent, f);
        fputc(CORNER, f);
        fputnc('-', width - 2, f);
        fputc(CORNER, f);
        fputc('\n', f);

        free(remaining);
        free(max);

        return true;
}

void table_free(struct table *t)
{
        if (t == NULL)
                return;

        if (t->data != NULL) {
                size_t i, j;

                for (i = 0; i < t->rows; ++i) {
                        if (t->data[i] != NULL) {
                            for (j = 0; j < t->cols; ++j) {
                                    if (t->data[i][j] != NULL) {
                                            free(t->data[i][j]);
                                    }
                            }
                            free(t->data[i]);
                        }
                }

                free(t->data);
        }

        if (t->max != NULL) {
                free(t->max);
        }

        if (t->align != NULL) {
                free(t->align);
        }
        if (t->headers != NULL) {
                free(t->headers);
        }
        if (t->fmt != NULL) {
                free(t->fmt);
        }

        memset(t, 0, sizeof(struct table));
}
