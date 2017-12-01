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

#ifndef __nx_format_base_h
#define __nx_format_base_h

#if _FILE_OFFSET_BITS != 64
#error Please add -D_FILE_OFFSET_BITS=64 to your compile flags!
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef int64_t  nx_ino_t;
typedef uint16_t nx_mode_t;
typedef uint32_t nx_nlink_t;
typedef uint32_t nx_uid_t;
typedef uint32_t nx_gid_t;
typedef uint32_t nx_dev_t;
typedef uint32_t nx_blksize_t;
typedef uint64_t nx_blkcnt_t;
typedef int64_t  nx_off_t;
typedef int64_t  nx_time_t;
typedef uint32_t nx_nseconds_t;

struct nx_timespec {
    nx_time_t     tv_sec;
    nx_nseconds_t tv_nsec;
};

#endif  /* !__nx_format_base_h */
