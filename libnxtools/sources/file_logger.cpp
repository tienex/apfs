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

#include "nxtools/file_logger.h"

#include <ctime>

using nxtools::file_logger;

static char const *
format_time(char *buf, size_t bufsize)
{
    time_t now = time(nullptr);
    struct tm *tm = localtime(&now);

    strftime(buf, bufsize, "%Y-%m-%d %H:%M:%S", tm);
    return buf;
}

file_logger::file_logger(char const *path)
{
    char buf[128];

    _fp = fopen(path, "a+t");
    if (_fp != nullptr) {
        fprintf(_fp, "======= LOG STARTED AT %s ======\n",
                format_time(buf, sizeof(buf)));
    }
}

file_logger::~file_logger()
{
    if (_fp != nullptr) {
        char buf[128];
        fprintf(_fp, "======= LOG ENDED AT %s ======\n",
                format_time(buf, sizeof(buf)));

        fclose(_fp);
    }
}

void file_logger::
log(nx::severity::value const &severity, char const *format, va_list ap)
{
    static char const * const severity_name[] = {
        "debug", "notice", "info", "warning", "error", "fatal", nullptr
    };

    char buf[128];

    fprintf(_fp, "[%s] %s: ", format_time(buf, sizeof(buf)),
            severity_name[severity]);
    vfprintf(_fp, format, ap);
    fputc('\n', _fp);
}
