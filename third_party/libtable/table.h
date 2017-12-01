#ifndef LIBTABLE_H_INCLUDED
#define LIBTABLE_H_INCLUDED

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

struct table {
        size_t cols;
        size_t rows;
        size_t alloc;
        size_t *max;
        char ***data;
        char **headers;
        char *align;
        char *fmt;
};

#define TABLE_LEFT   0
#define TABLE_RIGHT  1
#define TABLE_CENTER 2

#define TABLE_OPT_NOSEP 1

#ifdef __cplusplus
extern "C" {
#endif

bool table_vinit(struct table *t, va_list ap);
bool table_init(struct table *t, ...);
bool table_vadd(struct table *t, va_list ap);
bool table_add(struct table *t, ...);
bool table_print(struct table const *t, unsigned options, size_t indent,
        size_t maxwidth, FILE *f);
void table_free(struct table *t);

#ifdef __cplusplus
}
#endif

#endif
