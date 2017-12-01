#cmakedefine HAVE_SYS_XATTR_H
#cmakedefine HAVE_SYS_EXTATTR_H
#cmakedefine HAVE_SYS_DISK_H
#cmakedefine HAVE_SYS_DKIO_H
#cmakedefine HAVE_SYS_DISKLABEL_H
#cmakedefine HAVE_SYS_DISKLABEL32_H
#cmakedefine HAVE_SYS_DISKLABEL64_H
#cmakedefine HAVE_SYS_DISKSLICE_H
#cmakedefine HAVE_SYS_TIME_H
#cmakedefine HAVE_SYS_UTIME_H

#cmakedefine HAVE_SSIZE_T
#cmakedefine HAVE_TIMESPEC

#cmakedefine HAVE_GETOPT
#cmakedefine HAVE_FTRUNCATE
#cmakedefine HAVE_FUTIMENS
#cmakedefine HAVE_FUTIMES
#cmakedefine HAVE__FUTIME64
#cmakedefine HAVE__FUTIME
#ifndef __linux__
/* glibc defines fchflags, but it's unusable. */
#cmakedefine HAVE_FCHFLAGS
#endif
#cmakedefine HAVE_FSETXATTR
#cmakedefine HAVE_EXTATTR_SET_FD
#cmakedefine HAVE_EXTATTR_SET_FILE
#cmakedefine HAVE_ATTROPEN
#cmakedefine HAVE_SETEA
#cmakedefine HAVE__CHSIZE
#cmakedefine HAVE_CHSIZE
#cmakedefine HAVE_PREAD
#cmakedefine HAVE_PWRITE
#cmakedefine HAVE_SYMLINK
