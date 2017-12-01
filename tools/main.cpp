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

#include "nx/context.h"

#include "nxcompat/nxcompat.h"

#include <cstdio>
#include <cstdlib>

struct stderr_logger : nx::logger {
    void log(nx::severity::value const &severity, char const *format,
            va_list ap) override
    {
        static char const * const severity_name[] = {
            "debug", "notice", "info", "warning", "error", "fatal", nullptr
        };

        fprintf(stderr, "[%s] ", severity_name[severity]);
        vfprintf(stderr, format, ap);
        fputc('\n', stderr);
    }
};

extern int main_nx_scavenge(nx::context &context, int argc, char **argv);
extern int main_nx_omap(nx::context &context, int argc, char **argv);
extern int main_apfs_omap(nx::context &context, int argc, char **argv);
extern int main_apfs_traverse(nx::context &context, int argc, char **argv);
extern int main_apfs_content(nx::context &context, int argc, char **argv);
extern int main_apfs_extract(nx::context &context, int argc, char **argv);

int
main(int argc, char **argv)
{
    nx::context context;
    stderr_logger logger;

    context.set_logger(&logger);

    if (strstr(*argv, "nx_scavenge") != nullptr) {
        return main_nx_scavenge(context, argc, argv);
    } else if (strstr(*argv, "nx_omap") != nullptr) {
        return main_nx_omap(context, argc, argv);
    } else if (strstr(*argv, "apfs_omap") != nullptr) {
        return main_apfs_omap(context, argc, argv);
    } else if (strstr(*argv, "apfs_traverse") != nullptr) {
        return main_apfs_traverse(context, argc, argv);
    } else if (strstr(*argv, "apfs_content") != nullptr) {
        return main_apfs_content(context, argc, argv);
    } else if (strstr(*argv, "apfs_extract") != nullptr) {
        return main_apfs_extract(context, argc, argv);
    } else {
        fprintf(stderr, "error: you should not invoke '%s' directly.\n", *argv);
        exit(EXIT_FAILURE);
    }
}
