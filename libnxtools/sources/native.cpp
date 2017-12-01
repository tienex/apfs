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

#include "nxtools/native.h"
#include "nx/format/apfs.h"

#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/fs.h>
#endif

uint16_t nxtools::
apfs_mode_to_native(uint16_t mode)
{
    uint16_t native_mode = mode & 07777;

    switch (mode & APFS_INODE_MODE_IFMT) {
        case APFS_INODE_MODE_IFDIR:  native_mode |= S_IFDIR;  break;
        case APFS_INODE_MODE_IFREG:  native_mode |= S_IFREG;  break;
#ifdef S_IFLNK
        case APFS_INODE_MODE_IFLNK:  native_mode |= S_IFLNK;  break;
#else
        case APFS_INODE_MODE_IFLNK:  native_mode |= S_IFREG;  break;
#endif
        case APFS_INODE_MODE_IFIFO:  native_mode |= S_IFIFO;  break;
#ifdef S_IFSOCK
        case APFS_INODE_MODE_IFSOCK: native_mode |= S_IFSOCK; break;
#else
        case APFS_INODE_MODE_IFSOCK: native_mode |= S_IFIFO;  break;
#endif
#ifdef S_IFBLK
        case APFS_INODE_MODE_IFBLK:  native_mode |= S_IFBLK;  break;
#elif defined(S_IFCHR)
        case APFS_INODE_MODE_IFBLK:  native_mode |= S_IFCHR;  break;
#endif
#ifdef S_IFCHR
        case APFS_INODE_MODE_IFCHR:  native_mode |= S_IFCHR;  break;
#endif
    }

    return native_mode;
}

uint32_t nxtools::
apfs_bsd_flags_to_native(uint32_t bsd_flags)
{
    uint32_t native_flags = 0;

#if defined(__linux__)
    if (bsd_flags & APFS_INODE_BSD_FLAGS_USER_COMPRESSED) {
        native_flags |= FS_COMPR_FL;
    }
    if (bsd_flags & (APFS_INODE_BSD_FLAGS_USER_IMMUTABLE |
                 APFS_INODE_BSD_FLAGS_SUPER_IMMUTABLE)) {
        native_flags |= FS_IMMUTABLE_FL;
    }
    if (bsd_flags & (APFS_INODE_BSD_FLAGS_USER_APPEND |
                 APFS_INODE_BSD_FLAGS_SUPER_APPEND)) {
        native_flags |= FS_APPEND_FL;
    }
    if (bsd_flags & APFS_INODE_BSD_FLAGS_USER_NODUMP) {
        native_flags |= FS_NODUMP_FL;
    }
#else
#ifdef UF_NODUMP
    if (bsd_flags & APFS_INODE_BSD_FLAGS_USER_NODUMP) {
        native_flags |= UF_NODUMP;
    }
#endif
#ifdef UF_IMMUTABLE
    if (bsd_flags & APFS_INODE_BSD_FLAGS_USER_IMMUTABLE) {
        native_flags |= UF_IMMUTABLE;
    }
#endif
#ifdef UF_APPEND
    if (bsd_flags & APFS_INODE_BSD_FLAGS_USER_APPEND) {
        native_flags |= UF_IMMUTABLE;
    }
#endif
#ifdef UF_OPAQUE
    if (bsd_flags & APFS_INODE_BSD_FLAGS_USER_OPAQUE) {
        native_flags |= UF_OPAQUE;
    }
#endif
#ifdef UF_COMPRESSED
    if (bsd_flags & APFS_INODE_BSD_FLAGS_USER_COMPRESSED) {
        native_flags |= UF_COMPRESSED;
    }
#endif
#ifdef UF_TRACKED
    if (bsd_flags & APFS_INODE_BSD_FLAGS_USER_TRACKED) {
        native_flags |= UF_TRACKED;
    }
#endif
#ifdef UF_DATAVAULT
    if (bsd_flags & APFS_INODE_BSD_FLAGS_USER_DATAVAULT) {
        native_flags |= UF_DATAVAULT;
    }
#endif
#ifdef UF_HIDDEN
    if (bsd_flags & APFS_INODE_BSD_FLAGS_USER_HIDDEN) {
        native_flags |= UF_HIDDEN;
    }
#endif

#ifdef SF_ARCHIVED
    if (bsd_flags & APFS_INODE_BSD_FLAGS_SUPER_ARCHIVED) {
        native_flags |= SF_ARCHIVED;
    }
#endif
#ifdef SF_IMMUTABLE
    if (bsd_flags & APFS_INODE_BSD_FLAGS_SUPER_IMMUTABLE) {
        native_flags |= SF_IMMUTABLE;
    }
#endif
#ifdef SF_APPEND
    if (bsd_flags & APFS_INODE_BSD_FLAGS_SUPER_APPEND) {
        native_flags |= SF_APPEND;
    }
#endif
#ifdef SF_RESTRICTED
    if (bsd_flags & APFS_INODE_BSD_FLAGS_SUPER_RESTRICTED) {
        native_flags |= SF_RESTRICTED;
    }
#endif
#ifdef SF_NOUNLINK
    if (bsd_flags & APFS_INODE_BSD_FLAGS_SUPER_NOUNLINK) {
        native_flags |= SF_NOUNLINK;
    }
#endif
#endif

    return native_flags;
}
