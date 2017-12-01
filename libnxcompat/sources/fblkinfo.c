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

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#ifdef HAVE_SYS_DISK_H
#include <sys/disk.h>
#endif
#ifdef HAVE_SYS_DKIO_H
#include <sys/dkio.h>
#endif
#ifdef HAVE_SYS_DISKLABEL_H
#include <sys/disklabel.h>
#endif
#ifdef HAVE_SYS_DISKLABEL32_H
#include <sys/disklabel32.h>
#endif
#ifdef HAVE_SYS_DISKLABEL64_H
#include <sys/disklabel64.h>
#endif
#ifdef HAVE_SYS_DISKSLICE_H
#include <sys/diskslice.h>
#endif
#ifdef __linux__
#include <linux/fs.h>
#ifdef BLKGETSIZE64
#define BLKGETSIZEX BLKGETSIZE64
#define blk_size_t uint64_t
#else
#define BLKGETSIZEX BLKGETSIZE
#define blk_size_t uint32_t
#endif
#endif
#endif

int
fblkinfo(int fd, uint64_t *block_count, size_t *block_size)
{
#if defined(_WIN32)
    /*
     * Complicated mess!
     *
     * https://stackoverflow.com/questions/19825910/get-size-of-volume-on-windows
     */
    HANDLE hFile;
    DWORD  dwFileType;

    hFile = _get_osfhandle(fd);
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
        return -1;

    dwFileType = GetFileType(hFile);
    /* Is this a device? */
    if (dwFileType == FILE_TYPE_CHAR) {
        /*
         * Ok, let's the mess begin.
         *
         * Try the following methods:
         * - IOCTL_STORAGE_READ_CAPACITY
         * - IOCTL_DISK_GET_DRIVE_GEOMETRY (or _EX)
         * - IOCTL_DISK_GET_LENGTH_INFO (assuming always 512 bytes per sector)
         */
        DWORD                  bytesReturned;
#ifdef IOCTL_STORAGE_READ_CAPACITY
        STORAGE_READ_CAPACITY  src;
#endif
#ifdef IOCTL_DISK_GET_DRIVE_GEOMETRY_EX
        DISK_GEOMETRY_EX       dg;
#else
        DISK_GEOMETRY          dg;
#endif
#ifdef IOCTL_DISK_GET_LENGTH_INFO
        GET_LENGTH_INFORMATION gli;
#endif

#ifdef IOCTL_STORAGE_READ_CAPACITY
        if (DeviceIoControl(hFile, IOCTL_STORAGE_READ_CAPACITY, NULL, 0,
                    &src, sizeof(src), &bytesReturned, NULL)) {
            *block_count = src.NumberOfBlocks.QuadPart;
            *block_size = src.BlockLength;
            return 0;
        }
#endif

#ifdef IOCTL_DISK_GET_DRIVE_GEOMETRY_EX
        if (DeviceIoControl(hFile, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0,
                    &dg, sizeof(dg), &bytesReturned, NULL)) {
            *block_count = (dg.Geometry.Cylinders.QuadPart *
                            dg.Geometry.TracksPerCylinder *
                            dg.Geometry.SectorsPerTrack);
            *block_size = dg.Geometry.BytesPerSector;
            return 0;
        }
#else
        if (DeviceIoControl(hFile, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
                    &dg, sizeof(dg), &bytesReturned, NULL)) {
            *block_count = (dg.Geometry.Cylinders.QuadPart *
                            dg.Geometry.TracksPerCylinder *
                            dg.Geometry.SectorsPerTrack);
            *block_size = dg.Geometry.BytesPerSector;
            return 0;
        }
#endif

#ifdef IOCTL_DISK_GET_LENGTH_INFO
        if (DeviceIoControl(hFile, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0,
                    &gli, sizeof(gli), &bytesReturned, NULL)) {
            *block_count = (gli.Length.QuadPart + 511) / 512;
            *block_size = 512;
            return 0;
        }
#endif

        errno = ENOTSUP;
        return -1;
    }

    if (dwFileType == FILE_TYPE_DISK) {
        LARGE_INTEGER li;
        li.LowPart = GetFileSize(hFile, &li.Highpart);

        *block_count = (li.QuadPart + 511) / 512;
        *block_size  = 512;
        return 0;
    }

    errno = EINVAL;
    return -1;
#else
    struct stat stbuf;

    if (fstat(fd, &stbuf) < 0)
        return -1;

    switch (stbuf.st_mode & S_IFMT) {
        case S_IFCHR:
        case S_IFBLK:
            {
#ifdef __APPLE__
                uint64_t bcount;
                uint32_t bsize;
                if (ioctl(fd, DKIOCGETBLOCKSIZE, &bsize) < 0 ||
                    ioctl(fd, DKIOCGETBLOCKCOUNT, &bcount) < 0)
                    return -1;

                *block_count = bcount;
                *block_size = bsize;
#elif defined(DIOCGMEDIASIZE)
                u_int sectorsize;
                off_t mediasize;
                if (ioctl(fd, DIOCGSECTORSIZE, &sectorsize) < 0 ||
                    ioctl(fd, DIOCGMEDIASIZE, &mediasize) < 0)
                    return -1;

                *block_count = mediasize / sectorsize;
                *block_size = sectorsize;
#elif defined(DIOCGDINFO)
# if !defined(DL_GETPSIZE)
#  define DL_GETPSIZE(p) ((p)->p_bsize)
# endif
                struct disklabel dkl;
                if (ioctl(fd, DIOCGDINFO, &dkl) < 0)
                    return -1;

                *block_count = DL_GETPSIZE(&dkl.d_partitions[minor(stbuf.st_rdev) & 15]);
                *block_size = 512;
#elif defined(DIOCGPART)
                struct partinfo pi;

                if (ioctl(fd, DIOCGPART, &pi) < 0)
                    return -1;

                *block_count = pi.media_blocks;
                *block_size = pi.media_blksize;
#elif defined(DIOCGDINFO32) || defined(DIOCGDINFO64)
# if !defined(DL_GETPSIZE)
#  define DL_GETPSIZE(p) ((p)->p_bsize)
# endif
                struct disklabel64 dkl64;
                struct disklabel32 dkl32;

                if (ioctl(fd, DIOCGDINFO64, &dkl64) < 0) {
                    if (ioctl(fd, DIOCGDINFO32, &dkl32) < 0)
                        return -1;

                    *block_count = dkl32.d_partitions[minor(stbuf.st_rdev) & 15].p_size;
                } else {
                    *block_count = dkl64.d_partitions[minor(stbuf.st_rdev) & 15].p_bsize;
                }
                *block_size = 512;
#elif defined(__linux__)
                blk_size_t bcount;
                if (ioctl(fd, BLKGETSIZEX, &bcount) < 0)
                    return -1;

                *block_count = bcount;
                *block_size = 512;
#else
                errno = ENOTSUP;
                return -1;
#endif
            }
            break;

        case S_IFREG:
            *block_size = getpagesize();
            *block_count = (stbuf.st_size + *block_size - 1) / *block_size;
            break;

        default:
            errno = EINVAL;
            return -1;
    }

    return 0;
#endif
}
