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

#ifndef HAVE_FTRUNCATE

#ifdef _WIN32
#include <windows.h>
#endif

int
ftruncate(int fd, off_t offset)
{
#ifdef _WIN32
    HANDLE                hFile;
    FILE_END_OF_FILE_INFO feofi;

    hFile = (HANDLE)_get_osfhandle(fd);
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
        return -1;

    feofi.EndOfFile.QuadPart = offset;
    if (!SetFileInformationByHandle(hFile, FileEndOfFileInfo, &feofi,
                sizeof(feofi))) {
        errno = map_error_to_errno(GetLastError());
        return -1;
    }

    return 0;
#elif defined(HAVE__CHSIZE)
    return _chsize(fd, offset);
#elif defined(HAVE_CHSIZE)
    return chsize(fd, offset);
#else
    errno = ENOTSUP;
    return -1;
#endif
}

#endif
