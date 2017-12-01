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

#ifndef __APPLE__

#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/fs.h>
#endif

#include "nx/format/apfs.h"

/*
 * This does really translate macOS -> host flags (where applicable).
 */

int
fchflags_nx(int fd, int flags)
{
#ifdef HAVE_FCHFLAGS
    int native_flags = 0;

#ifdef UF_NODUMP
    if (flags & APFS_INODE_BSD_FLAGS_USER_NODUMP) {
        native_flags |= UF_NODUMP;
    }
#endif
#ifdef UF_IMMUTABLE
    if (flags & APFS_INODE_BSD_FLAGS_USER_IMMUTABLE) {
        native_flags |= UF_IMMUTABLE;
    }
#endif
#ifdef UF_APPEND
    if (flags & APFS_INODE_BSD_FLAGS_USER_APPEND) {
        native_flags |= UF_IMMUTABLE;
    }
#endif
#ifdef UF_OPAQUE
    if (flags & APFS_INODE_BSD_FLAGS_USER_OPAQUE) {
        native_flags |= UF_OPAQUE;
    }
#endif
#ifdef UF_COMPRESSED
    if (flags & APFS_INODE_BSD_FLAGS_USER_COMPRESSED) {
        native_flags |= UF_COMPRESSED;
    }
#endif
#ifdef UF_TRACKED
    if (flags & APFS_INODE_BSD_FLAGS_USER_TRACKED) {
        native_flags |= UF_TRACKED;
    }
#endif
#ifdef UF_DATAVAULT
    if (flags & APFS_INODE_BSD_FLAGS_USER_DATAVAULT) {
        native_flags |= UF_DATAVAULT;
    }
#endif
#ifdef UF_HIDDEN
    if (flags & APFS_INODE_BSD_FLAGS_USER_HIDDEN) {
        native_flags |= UF_HIDDEN;
    }
#endif

#ifdef SF_ARCHIVED
    if (flags & APFS_INODE_BSD_FLAGS_SUPER_ARCHIVED) {
        native_flags |= SF_ARCHIVED;
    }
#endif
#ifdef SF_IMMUTABLE
    if (flags & APFS_INODE_BSD_FLAGS_SUPER_IMMUTABLE) {
        native_flags |= SF_IMMUTABLE;
    }
#endif
#ifdef SF_APPEND
    if (flags & APFS_INODE_BSD_FLAGS_SUPER_APPEND) {
        native_flags |= SF_APPEND;
    }
#endif
#ifdef SF_RESTRICTED
    if (flags & APFS_INODE_BSD_FLAGS_SUPER_RESTRICTED) {
        native_flags |= SF_RESTRICTED;
    }
#endif
#ifdef SF_NOUNLINK
    if (flags & APFS_INODE_BSD_FLAGS_SUPER_NOUNLINK) {
        native_flags |= SF_NOUNLINK;
    }
#endif

    return fchflags(fd, native_flags);
#elif defined(__linux__)
    int native_flags = 0;

    if (ioctl(fd, FS_IOC_GETFLAGS, &native_flags) < 0)
        return -1;

    native_flags &= ~(FS_COMPR_FL | FS_IMMUTABLE_FL |
                      FS_APPEND_FL | FS_NODUMP_FL);

    if (flags & APFS_INODE_BSD_FLAGS_USER_COMPRESSED) {
        native_flags |= FS_COMPR_FL;
    }
    if (flags & (APFS_INODE_BSD_FLAGS_USER_IMMUTABLE |
                 APFS_INODE_BSD_FLAGS_SUPER_IMMUTABLE)) {
        native_flags |= FS_IMMUTABLE_FL;
    }
    if (flags & (APFS_INODE_BSD_FLAGS_USER_APPEND |
                 APFS_INODE_BSD_FLAGS_SUPER_APPEND)) {
        native_flags |= FS_APPEND_FL;
    }
    if (flags & APFS_INODE_BSD_FLAGS_USER_NODUMP) {
        native_flags |= FS_NODUMP_FL;
    }

    if (ioctl(fd, FS_IOC_SETFLAGS, &native_flags) < 0)
        return -1;

    return 0;
#else
    errno = ENOTSUP;
    return -1;
#endif
}

#endif
