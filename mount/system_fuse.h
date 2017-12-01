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

#ifndef __system_fuse_h
#define __system_fuse_h

#include <fuse.h>

#ifdef __APPLE__
#include <unistd.h>
#endif

//
// Ensure FUSE_MAKE_VERSION and FUSE_VERSION exist.
//
#ifndef FUSE_MAKE_VERSION
#define FUSE_MAKE_VERSION(x, y) ((x) * 10 + (y))
#endif
#ifndef FUSE_VERSION
#define FUSE_VERSION FUSE_MAKE_VERSION(FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION)
#endif

#define FUSE_VERSION_LT(x, y) (FUSE_VERSION < FUSE_MAKE_VERSION(x, y))

//
// Handle getxattr Apple's differences.
//
#ifdef __APPLE__
#define POSITION_ARG , uint32_t position
#define POSITION_DECL
#else
#define POSITION_ARG
#define POSITION_DECL uint32_t position = 0
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
//
// NetBSD, OpenBSD and DragonFlyBSD do not call readdir multiple times.
//
#define FUSE_READDIR_SINGLE_READ
#endif

#endif  /* !__system_fuse_h */
