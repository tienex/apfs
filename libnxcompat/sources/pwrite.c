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

#include "nxcompat/nxcompat.h"

#ifndef HAVE_PWRITE

#ifdef _WIN32
#include <windows.h>
#endif

ssize_t
pwrite(int fd, void const *buf, size_t count, off_t offset)
{
#if defined(_WIN32)
    HANDLE     hFile;
    OVERLAPPED overlapped;
    DWORD      numberOfBytesWritten;

    hFile = (HANDLE)_get_osfhandle(fd);
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
        return -1;

    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.Offset     = offset & 0xffffffff;
    overlapped.OffsetHigh = (uint64_t)offset >> 32;
    if (!WriteFile(hFile, buf, count, &numberOfBytesWritten, &overlapped)) {
        errno = map_error_to_errno(GetLastError());
        return -1;
    }
    return numberOfBytesWritten;
#else
    off_t   old;
    ssize_t nwritten;
    int     error;

    old = lseek(fd, 0, SEEK_CUR);
    if (old < 0)
        return -1;

    if (lseek(fd, offset, SEEK_SET) < 0) {
        error = errno;
        lseek(fd, old, SEEK_SET);
        errno = error;
        return -1;
    }

    nwritten = write(fd, buf, count);
    error = errno;
    lseek(fd, old, SEEK_SET);
    errno = error;

    return nwritten;
#endif
}

#endif
