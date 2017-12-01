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

#include "nxtools/time.h"

struct timespec nxtools::
ns_to_timespec(uint64_t ns)
{
    struct timespec tv;
    tv.tv_sec  = ns / 1000000000;
    tv.tv_nsec = ns % 1000000000;
    return tv;
}

struct nx_timespec nxtools::
ns_to_nx_timespec(uint64_t ns)
{
    struct nx_timespec tv;
    tv.tv_sec  = ns / 1000000000;
    tv.tv_nsec = ns % 1000000000;
    return tv;
}

struct timespec nxtools::
nx_timespec_to_timespec(struct nx_timespec ts)
{
    struct timespec tv;
    tv.tv_sec  = ts.tv_sec;
    tv.tv_nsec = ts.tv_nsec;
    return tv;
}
