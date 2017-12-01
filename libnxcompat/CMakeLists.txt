# Copyright (c) 2017-present Orlando Bassotto
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

project(libnxcompat)

cmake_minimum_required(VERSION 3.5)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckTypeSize)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include_directories(${CMAKE_SOURCE_DIR}/libnx/headers
                    ${CMAKE_CURRENT_SOURCE_DIR}/headers
                    ${CMAKE_CURRENT_BINARY_DIR}/headers)

CHECK_INCLUDE_FILES("sys/types.h;sys/xattr.h" HAVE_SYS_XATTR_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/extattr.h" HAVE_SYS_EXTATTR_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/disk.h" HAVE_SYS_DISK_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/dkio.h" HAVE_SYS_DKIO_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/disklabel.h" HAVE_SYS_DISKLABEL_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/disklabel32.h" HAVE_SYS_DISKLABEL32_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/disklabel64.h" HAVE_SYS_DISKLABEL64_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/diskslice.h" HAVE_SYS_DISKSLICE_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/time.h" HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/utime.h" HAVE_SYS_UTIME_H)

set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h")
CHECK_TYPE_SIZE("ssize_t" SSIZE_T)

if (HAVE_SYS_TIME_H)
    set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;sys/time.h;time.h")
else ()
    set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;time.h")
endif ()
CHECK_TYPE_SIZE("struct timespec" TIMESPEC)

unset(CMAKE_EXTRA_INCLUDE_FILES)

CHECK_FUNCTION_EXISTS(getopt HAVE_GETOPT)
CHECK_FUNCTION_EXISTS(ftruncate HAVE_FTRUNCATE)
CHECK_FUNCTION_EXISTS(futimens HAVE_FUTIMENS)
CHECK_FUNCTION_EXISTS(futimens HAVE_FUTIMES)
CHECK_FUNCTION_EXISTS(_futime64 HAVE__FUTIME64)
CHECK_FUNCTION_EXISTS(_futime HAVE__FUTIME)
CHECK_FUNCTION_EXISTS(fchflags HAVE_FCHFLAGS)
CHECK_FUNCTION_EXISTS(fsetxattr HAVE_FSETXATTR)
CHECK_FUNCTION_EXISTS(extattr_set_fd HAVE_EXTATTR_SET_FD)
CHECK_FUNCTION_EXISTS(extattr_set_file HAVE_EXTATTR_SET_FILE)
CHECK_FUNCTION_EXISTS(attropen HAVE_ATTROPEN)
CHECK_FUNCTION_EXISTS(fsetea HAVE_FSETEA)
CHECK_FUNCTION_EXISTS(_chsize HAVE__CHSIZE)
CHECK_FUNCTION_EXISTS(chsize HAVE_CHSIZE)
CHECK_FUNCTION_EXISTS(pread HAVE_PREAD)
CHECK_FUNCTION_EXISTS(pwrite HAVE_PWRITE)
CHECK_FUNCTION_EXISTS(symlink HAVE_SYMLINK)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/headers/nxcompat/nxcompat_config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/headers/nxcompat/nxcompat_config.h)

if (UNIX)
    add_definitions(-D_GNU_SOURCE)
endif ()

add_library(nxcompat STATIC
            sources/fblkinfo.c
            sources/fchflags.c
            sources/fsetxattr.c
            sources/ftruncate.c
            sources/futimens.c
            sources/getopt.c
            sources/pread.c
            sources/pwrite.c
            sources/symlink.c
            sources/winerror.c
            sources/third_party/base64.c
            sources/third_party/ConvertUTF.c)
set_target_properties(nxcompat PROPERTIES
                      POSITION_INDEPENDENT_CODE ON
                      C_VISIBILITY_PRESET hidden)
if (WIN32)
    target_link_libraries(nxcompat kernel32 advapi32 ntdll)
endif ()

install(TARGETS nxcompat
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
