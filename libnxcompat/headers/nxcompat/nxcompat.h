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

#ifndef __nxcompat_nxcompat_h
#define __nxcompat_nxcompat_h

#include "nxcompat/nxcompat_config.h"
#include "nxcompat/third_party/ConvertUTF.h"
#include "nxcompat/third_party/base64.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif
#ifdef HAVE_SYS_EXTATTR_H
#include <sys/extattr.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <errno.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>

#ifndef PRIuSIZE
#if defined(_MSC_VER)
#if defined(_WIN64)
#define PRIdSIZE "I64d"
#define PRIiSIZE "I64i"
#define PRIoSIZE "I64o"
#define PRIuSIZE "I64u"
#define PRIxSIZE "I64x"
#define PRIXSIZE "I64X"
#else
#define PRIdSIZE "d"
#define PRIiSIZE "i"
#define PRIoSIZE "o"
#define PRIuSIZE "u"
#define PRIxSIZE "x"
#define PRIXSIZE "X"
#endif
#else
#define PRIdSIZE "zd"
#define PRIiSIZE "zi"
#define PRIoSIZE "zo"
#define PRIuSIZE "zu"
#define PRIxSIZE "zx"
#define PRIXSIZE "zX"
#endif
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#ifndef HAVE_SSIZE_T
#if defined(_WIN64)
typedef __int64 ssize_t;
#elif defined(_WIN32)
typedef long ssize_t;
#else
typedef int ssize_t;
#endif
#endif

#ifndef ENOATTR
#ifdef ENODATA
#define ENOATTR ENODATA
#else
#define ENOATTR ENOENT
#endif
#endif

#ifndef HAVE_TIMESPEC
struct timespec {
    time_t tv_sec;
    uint32_t tv_nsec;
};
#endif

#ifndef HAVE_GETOPT
extern int optind;
extern char *optarg;
#endif

#ifdef __cplusplus
extern "C" {
#endif

int fblkinfo(int fd, uint64_t *block_count, size_t *block_size);

#ifndef HAVE_GETOPT
int getopt(int argc, char * const argv[], char const *optstring);
#endif

#ifndef HAVE_FTRUNCATE
int ftruncate(int fd, off_t offset);
#endif

#ifndef HAVE_FUTIMENS
int futimens_nx(int fd, struct timespec const times[2]);
#else
#define futimens_nx futimens
#endif

#ifdef __APPLE__
#define fchflags_nx fchflags
#else
int fchflags_nx(int fd, int flags);
#endif

#ifndef HAVE_PREAD
ssize_t pread(int fd, void *buf, size_t count, off_t offset);
#endif

#ifndef HAVE_PWRITE
ssize_t pwrite(int fd, void const *buf, size_t count, off_t offset);
#endif

#ifdef __APPLE__
#define fsetxattr_nx(fd, name, value, size, options) \
    fsetxattr(fd, name, value, size, 0, options)
#elif !defined(HAVE_FSETXATTR) || defined(__linux__)
#ifndef HAVE_FSETXATTR
#define XATTR_CREATE  1
#define XATTR_REPLACE 2
#endif
#ifdef _WIN32
#define XATTR_EA      0x0000 /* Use Extended Attributes */
#define XATTR_ADS     0x8000 /* Use Alternate Data Streams */
#endif
int fsetxattr_nx(int fd, const char *name, void const *value, size_t size,
        int options);
#else
#define fsetxattr_nx fsetxattr
#endif

#ifndef HAVE_SYMLINK
int symlink_nx(char const *target, char const *source);
#else
#define symlink_nx symlink
#endif

#ifdef _WIN32
int map_error_to_errno(DWORD dwError);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* !__nxcompat_nxcompat_h */
