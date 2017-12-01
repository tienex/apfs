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

#if !defined(HAVE_FSETXATTR) || defined(__linux__)

#ifdef _WIN32
#include <windows.h>
#include <ntddk.h>
#endif

#include <stdio.h>
#include <stdlib.h>

int
fsetxattr_nx(int fd, const char *name, void const *value, size_t size,
        int options)
{
#if defined(_WIN32)
    HANDLE                    hFile;
    FILE_GET_EA_INFORMATION  *pEAGetInfo;
    FILE_FULL_EA_INFORMATION *pEAFullInfo;
    IO_STATUS_BLOCK           ioStatusBlock;
    ULONG                     ulEAFullLength;
    ULONG                     ulEAGetLength;
    BOOL                      fExists;
    NTSTATUS                  status;

    if (options == 0 ||
        (options & (XATTR_CREATE | XATTR_REPLACE)) ==
            (XATTR_CREATE | XATTR_REPLACE)) {
        errno = EINVAL;
        return -1;
    }

    hFile = _get_osfhandle(fd);
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
        return -1;

    /*
     * Allocate the get and full ea descriptors.
     */
    ulEAGetLength = sizeof(FILE_GET_EA_INFORMATION) + strlen(name);
    pEAGetInfo = (FILE_GET_EA_INFORMATION *)calloc(1, ulEAGetLength);
    if (pEAGetInfo == NULL)
        return -1;

    ulEAFullLength = sizeof(FILE_FULL_EA_INFORMATION) + strlen(name) + size;
    pEAFullInfo = (FILE_FULL_EA_INFORMATION *)calloc(1, ulEAFullLength);
    if (pEAFullInfo == NULL) {
        free(pEAGetInfo);
        return -1;
    }

    /*
     * Prepare get descriptor.
     */
    strcpy(pEAGetInfo->EaName, name);
    pEAGetInfo->EaNameLength = strlen(name);

    /*
     * Query the EA.
     */
    status = NtQueryEaFile(hFile, &ioStatusBlock, pEAFullInfo, ulEAFullLength,
            FALSE, pEAGetInfo, ulEAGetLength, NULL, FALSE);

    free(pEAGetInfo);

    if (status != STATUS_SUCCESS) {
        free(pEAFullInfo);
        goto fail;
    }

    fExists = (pEAFullInfo->EaValueLength != 0);

    if ((fExists && (options & XATTR_CREATE) != 0) ||
        (!fExists && (options & XATTR_REPLACE) != 0)) {
        free(pEAFullInfo);
        errno = fExists ? EEXIST : ENOATTR;
        return -1;
    }

    /*
     * Prepare full descriptor.
     */
    strcpy(pEAFullInfo->EaName, name);
    pEAFullInfo->EaNameLength = strlen(name);
    pEAFullInfo->EaValueLength = size;
    memcpy(pEAFullInfo->EaName + pEAFullInfo->EaNameLength + 1, value, size);

    status = NtSetEaFile(hFile, &ioStatusBlock, pEAFullInfo, ulEAFullLength);

    free(pEAFullInfo);

    if (status != NT_SUCCESS)
        goto fail;

    return 0;

fail:
    switch (status) {
        case STATUS_EAS_NOT_SUPPORTED:
            errno = ENOTSUP;
            break;
        case STATUS_INSUFFICIENT_RESOURCES:
            errno = ENOMEM;
            break;
        case STATUS_EA_LIST_INCONSISTENT:
        default:
            errno = EIO;
            break;
    }
    return -1;
#elif defined(HAVE_EXTATTR_SET_FD) || defined(HAVE_EXTATTR_SET_FILE)
    int exists;
    char c;
#ifndef HAVE_EXTATTR_SET_FD
    char path[64];

    snprintf(path, sizeof(path), "/dev/fd/%d", fd);
#endif

    if (options == 0 ||
        (options & (XATTR_CREATE | XATTR_REPLACE)) ==
            (XATTR_CREATE | XATTR_REPLACE)) {
        errno = EINVAL;
        return -1;
    }

#ifdef HAVE_EXTATTR_SET_FD
    exists = (extattr_get_fd(fd, EXTATTR_NAMESPACE_USER, name,
                &c, sizeof(c)) == sizeof(c));
#else
    exists = (extattr_get_file(path, EXTATTR_NAMESPACE_USER, name,
                &c, sizeof(c)) == sizeof(c));
#endif

    if ((exists && (options & XATTR_CREATE) != 0) ||
        (!exists && (options & XATTR_REPLACE) != 0)) {
        errno = exists ? EEXIST : ENOATTR;
        return -1;
    }

    if (options & XATTR_REPLACE) {
#ifdef HAVE_EXTATTR_SET_FD
        if (extattr_delete_fd(fd, EXTATTR_NAMESPACE_USER, name) < 0)
            return -1;
#else
        if (extattr_delete_file(path, EXTATTR_NAMESPACE_USER, name) < 0)
            return -1;
#endif
    }

#ifdef HAVE_EXTATTR_SET_FD
    return (extattr_set_fd(fd, EXTATTR_NAMESPACE_USER, name,
                value, size) == size) ? 0 : -1;
#else
    return (extattr_set_file(path, EXTATTR_NAMESPACE_USER, name,
                value, size) == size) ? 0 : -1;
#endif
#elif defined(HAVE_ATTROPEN)
    int exists, attrfd, oflags = O_CREAT;
    ssize_t nwritten;
    char path[64];
    struct stat st;

    if (fstat(fd, &st) < 0)
        return -1;

    snprintf(path, sizeof(path), "/dev/fd/%d", fd);

    if (options == 0 ||
        (options & (XATTR_CREATE | XATTR_REPLACE)) ==
            (XATTR_CREATE | XATTR_REPLACE)) {
        errno = EINVAL;
        return -1;
    }

    close(attrfd = attropen(path, name, O_RDONLY));
    exists = !(attrfd < 0);

    if ((exists && (options & XATTR_CREATE) != 0) ||
        (!exists && (options & XATTR_REPLACE) != 0)) {
        errno = exists ? EEXIST : ENOATTR;
        return -1;
    }

    if (options & XATTR_REPLACE) {
        oflags = O_TRUNC;
    }

    attrfd = attropen(path, name, oflags | O_WRONLY, st.st_mode & ~0111);
    if (attrfd < 0)
        return -1;

    nwritten = write(attrfd, value, size);
    close(attrfd);

    return (nwritten == size) ? 0 : -1;
#elif defined(HAVE_FSETEA)
    int exists;
    char c;

    if (options == 0 ||
        (options & (XATTR_CREATE | XATTR_REPLACE)) ==
            (XATTR_CREATE | XATTR_REPLACE)) {
        errno = EINVAL;
        return -1;
    }

    exists = (fgetea(fd, name, &c, sizeof(c)) == sizeof(c));

    if ((exists && (options & XATTR_CREATE) != 0) ||
        (!exists && (options & XATTR_REPLACE) != 0)) {
        errno = exists ? EEXIST : ENOATTR;
        return -1;
    }

    if (options & XATTR_REPLACE) {
        if (fremoveea(fd, name) < 0)
            return -1;
    }

    return (fsetea(fd, name, value, size, 0) == size) ? 0 : -1;
#elif defined(__linux__)
    /*
     * Need to prepend user.
     */
    int rc;
    char *new_name;
    if (asprintf(&new_name, "user.%s", name) < 0) {
        errno = ENOMEM;
        return -1;
    }

    rc = fsetxattr(fd, new_name, value, size, options);
    free(new_name);

    return rc;
#else
    errno = ENOTSUP;
    return -1;
#endif
}

#endif
