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

#ifndef HAVE_SYMLINK

#ifdef _WIN32
#include <windows.h>
#endif

int
symlink_nx(char const *target, char const *source)
{
#if defined(_WIN32)
    struct stat st;
    DWORD       dwFlags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

    if (stat(source, &st) == 0) {
        if (st.st_mode & _S_IFDIR) {
            dwFlags |= SYMBOLIC_LINK_FLAG_DIRECTORY
        }
    }

    if (!CreateSymbolicLink(target, source, dwFlags)) {
        errno = map_error_to_errno(GetLastError());
        return -1;
    }

    return 0;
#else
    errno = ENOTSUP;
    return -1;
#endif
}

#endif
