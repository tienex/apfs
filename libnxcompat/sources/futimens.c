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

#ifndef HAVE_FUTIMENS

#ifdef _WIN32
#include <windows.h>

#define TICKS_PER_SECOND    1000000000ULL
#define SEC_TO_UNIX_EPOCH   11644473600ULL
#define TICKS_TO_UNIX_EPOCH (TICKS_PER_SECOND * SEC_TO_UNIX_EPOCH)

static void
timespec_to_FILETIME(struct timespec const *ts, FILETIME *filetime)
{
    uint64_t nsec = ts->tv_sec * TICKS_PER_SECOND + ts->tv_nsec;
    uint64_t ticks = nsec / NANOSECONDS_PER_TICK + TICKS_TO_UNIX_EPOCH;
    filetime->dwLowDateTime = (DWORD)(ticks % 0x100000000ULL);
    filetime->dwHighDateTime = (DWORD)(ticks / 0x100000000ULL);
}
#endif

#ifdef HAVE_SYS_UTIME_H
#include <sys/utime.h>
#endif

int
futimens_nx(int fd, struct timespec const times[3])
{
#if defined(_WIN32)
    HANDLE   hFile;
    FILETIME ftAccess;
    FILETIME ftModification;
    FILETIME ftCreation;

    timespec_to_FILETIME(&times[0], &ftAccess);
    timespec_to_FILETIME(&times[1], &ftModification);
    timespec_to_FILETIME(&times[2], &ftCreation);

    hFile = _get_osfhandle(fd);
    if (!SetFileTime(hFile, &ftCreation, &ftAccess, &ftModification)) {
        errno = map_error_to_errno(GetLastError());
        return -1;
    }

    return 0;
#elif defined(HAVE_FUTIMES)
    struct timeval ustimes[2];

    ustimes[0].tv_sec  = times[0].tv_sec;
    ustimes[0].tv_usec = (times[0].tv_nsec + 999) / 1000;

    ustimes[1].tv_sec  = times[1].tv_sec;
    ustimes[1].tv_usec = (times[1].tv_nsec + 999) / 1000;

    return futimes(fd, ustimes);
#elif defined(HAVE__FUTIME64)
    struct _utimbuf64 utb;
    utb.actime = times[0].tv_sec;
    utb.modtime = times[1].tv_sec;
    return _futime64(fd, &utb);
#elif defined(HAVE__FUTIME)
    struct _utimbuf utb;
    utb.actime = times[0].tv_sec;
    utb.modtime = times[1].tv_sec;
    return _futime(fd, &utb);
#elif defined(HAVE__FUTIME)
    struct _utimbuf utb;
    utb.actime = times[0].tv_sec;
    utb.modtime = times[1].tv_sec;
    return _futime(fd, &utb);
#else
    errno = ENOSYS;
    return -1;
#endif
}

#endif
