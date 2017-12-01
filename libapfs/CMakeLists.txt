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

project(libapfs)

cmake_minimum_required(VERSION 3.5)

if (APPLE AND POLICY CMP0068)
    cmake_policy(SET CMP0068 NEW)
endif ()

if (CMAKE_BUILD_TYPE MATCHES "Release" AND POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)

    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED)
endif ()

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckStructHasMember)
include(CheckTypeSize)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 11)

include_directories(headers
                    ${CMAKE_CURRENT_BINARY_DIR}/headers
                    ${CMAKE_SOURCE_DIR}/libnx/headers
                    ${CMAKE_SOURCE_DIR}/libnxtools/headers
                    ${CMAKE_SOURCE_DIR}/libnxcompat/headers
                    ${CMAKE_BINARY_DIR}/libnxcompat/headers)

CHECK_INCLUDE_FILES("sys/types.h;sys/param.h" HAVE_SYS_PARAM_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/mkdev.h" HAVE_SYS_MKDEV_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/mount.h" HAVE_SYS_MOUNT_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/statfs.h" HAVE_SYS_STATFS_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/statvfs.h" HAVE_SYS_STATVFS_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/sysmacros.h" HAVE_SYS_SYSMACROS_H)
CHECK_INCLUDE_FILES("sys/types.h;sys/vfs.h" HAVE_SYS_VFS_H)

set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h")
if (HAVE_SYS_PARAM_H)
    set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};sys/param.h")
endif ()
if (HAVE_SYS_MOUNT_H)
    set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};sys/mount.h")
endif ()
if (HAVE_SYS_STATFS_H)
    set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};sys/statfs.h")
endif ()
if (HAVE_SYS_STATVFS_H)
    set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};sys/statvfs.h")
endif ()
if (HAVE_SYS_VFS_H)
    set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};sys/vfs.h")
endif ()

CHECK_TYPE_SIZE("struct statfs" STATFS)
CHECK_TYPE_SIZE("struct statvfs" STATVFS)

unset(CMAKE_EXTRA_INCLUDE_FILES)

if (HAVE_STATFS)
    if (HAVE_SYS_MOUNT_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_bavail "sys/mount.h" HAVE_STATFS_F_BAVAIL_IN_SYS_MOUNT_H LANGUAGE C)
    endif ()
    if (HAVE_SYS_STATFS_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_bavail "sys/statfs.h" HAVE_STATFS_F_BAVAIL_IN_SYS_STATFS_H LANGUAGE C)
    endif ()
    if (HAVE_SYS_VFS_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_bavail "sys/vfs.h" HAVE_STATFS_F_BAVAIL_IN_SYS_VFS_H LANGUAGE C)
    endif ()

    if (HAVE_SYS_MOUNT_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_iosize "sys/mount.h" HAVE_STATFS_F_IOSIZE_IN_SYS_MOUNT_H LANGUAGE C)
    endif ()
    if (HAVE_SYS_STATFS_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_iosize "sys/statfs.h" HAVE_STATFS_F_IOSIZE_IN_SYS_STATFS_H LANGUAGE C)
    endif ()
    if (HAVE_SYS_VFS_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_iosize "sys/vfs.h" HAVE_STATFS_F_IOSIZE_IN_SYS_VFS_H LANGUAGE C)
    endif ()

    if (HAVE_SYS_MOUNT_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_frsize "sys/mount.h" HAVE_STATFS_F_FRSIZE_IN_SYS_MOUNT_H LANGUAGE C)
    endif ()
    if (HAVE_SYS_STATFS_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_frsize "sys/statfs.h" HAVE_STATFS_F_FRSIZE_IN_SYS_STATFS_H LANGUAGE C)
    endif ()
    if (HAVE_SYS_VFS_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_frsize "sys/vfs.h" HAVE_STATFS_F_FRSIZE_IN_SYS_VFS_H LANGUAGE C)
    endif ()

    if (HAVE_SYS_MOUNT_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_flag "sys/mount.h" HAVE_STATFS_F_FLAG_IN_SYS_MOUNT_H LANGUAGE C)
    endif ()
    if (HAVE_SYS_STATFS_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_flag "sys/statfs.h" HAVE_STATFS_F_FLAG_IN_SYS_STATFS_H LANGUAGE C)
    endif ()
    if (HAVE_SYS_VFS_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_flag "sys/vfs.h" HAVE_STATFS_F_FLAG_IN_SYS_VFS_H LANGUAGE C)
    endif ()

    if (HAVE_SYS_MOUNT_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_flags "sys/mount.h" HAVE_STATFS_F_FLAGS_IN_SYS_MOUNT_H LANGUAGE C)
    endif ()
    if (HAVE_SYS_STATFS_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_flags "sys/statfs.h" HAVE_STATFS_F_FLAGS_IN_SYS_STATFS_H LANGUAGE C)
    endif ()
    if (HAVE_SYS_VFS_H)
        CHECK_STRUCT_HAS_MEMBER("struct statfs" f_flags "sys/vfs.h" HAVE_STATFS_F_FLAGS_IN_SYS_VFS_H LANGUAGE C)
    endif ()

    if (HAVE_STATFS_F_BAVAIL_IN_SYS_MOUNT_H OR HAVE_STATFS_F_BAVAIL_IN_SYS_STATFS_H OR HAVE_STATFS_F_BAVAIL_IN_SYS_VFS_H)
        set(HAVE_STATFS_F_BAVAIL TRUE)
    endif ()

    if (HAVE_STATFS_F_IOSIZE_IN_SYS_MOUNT_H OR HAVE_STATFS_F_IOSIZE_IN_SYS_STATFS_H OR HAVE_STATFS_F_IOSIZE_IN_SYS_VFS_H)
        set(HAVE_STATFS_F_IOSIZE TRUE)
    endif ()

    if (HAVE_STATFS_F_FRSIZE_IN_SYS_MOUNT_H OR HAVE_STATFS_F_FRSIZE_IN_SYS_STATFS_H OR HAVE_STATFS_F_FRSIZE_IN_SYS_VFS_H)
        set(HAVE_STATFS_F_FRSIZE TRUE)
    endif ()

    if (HAVE_STATFS_F_FLAG_IN_SYS_MOUNT_H OR HAVE_STATFS_F_FLAG_IN_SYS_STATFS_H OR HAVE_STATFS_F_FLAG_IN_SYS_VFS_H)
        set(HAVE_STATFS_F_FLAG TRUE)
    endif ()

    if (HAVE_STATFS_F_FLAGS_IN_SYS_MOUNT_H OR HAVE_STATFS_F_FLAGS_IN_SYS_STATFS_H OR HAVE_STATFS_F_FLAGS_IN_SYS_VFS_H)
        set(HAVE_STATFS_F_FLAGS TRUE)
    endif ()
endif ()

if (HAVE_STATVFS)
    CHECK_STRUCT_HAS_MEMBER("struct statvfs" f_flag "sys/statvfs.h" HAVE_STATVFS_F_FLAG LANGUAGE C)
    CHECK_STRUCT_HAS_MEMBER("struct statvfs" f_flags "sys/statvfs.h" HAVE_STATVFS_F_FLAGS LANGUAGE C)
endif ()

set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h")
CHECK_STRUCT_HAS_MEMBER("struct stat" st_mtim "sys/stat.h" HAVE_STAT_ST_MTIM LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_mtimensec "sys/stat.h" HAVE_STAT_ST_MTIMENSEC LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_mtimespec "sys/stat.h" HAVE_STAT_ST_MTIMESPEC LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_birthtime "sys/stat.h" HAVE_STAT_ST_BIRTHTIME LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" __st_birthtime "sys/stat.h" HAVE_STAT___ST_BIRTHTIME LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_birthtimensec "sys/stat.h" HAVE_STAT_ST_BIRTHTIMENSEC LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" __st_birthtimensec "sys/stat.h" HAVE_STAT_ST___BIRTHTIMENSEC LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_birthtim "sys/stat.h" HAVE_STAT_ST_BIRTHTIM LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" __st_birthtim "sys/stat.h" HAVE_STAT___ST_BIRTHTIM LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_birthtimespec "sys/stat.h" HAVE_STAT_ST_BIRTHTIMESPEC LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_blksize "sys/stat.h" HAVE_STAT_ST_BLKSIZE LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_blocks "sys/stat.h" HAVE_STAT_ST_BLOCKS LANGUAGE C)
CHECK_STRUCT_HAS_MEMBER("struct stat" st_flags "sys/stat.h" HAVE_STAT_ST_FLAGS LANGUAGE C)
unset(CMAKE_EXTRA_INCLUDE_FILES)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/headers/apfs/libapfs_config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/headers/apfs/libapfs_config.h)

set(libapfs_SOURCES
    sources/internal/directory.cpp
    sources/internal/file.cpp
    sources/internal/object.cpp
    sources/internal/object_cache.cpp
    sources/internal/xattr.cpp
    sources/session.cpp
    sources/volume.cpp
    sources/object.cpp
    )

add_library(apfs_static STATIC ${libapfs_SOURCES})
target_link_libraries(apfs_static nx_static nxtools)
set_target_properties(apfs_static PROPERTIES OUTPUT_NAME "apfs")

add_library(apfs_shared SHARED ${libapfs_SOURCES})
target_link_libraries(apfs_shared nx_shared nxtools)
set_target_properties(apfs_shared PROPERTIES OUTPUT_NAME "apfs")
set_target_properties(apfs_shared PROPERTIES VERSION ${NXAPFS_VERSION})
if (IPO_SUPPORTED)
    set_target_properties(apfs_shared PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

install(FILES
        headers/apfs/internal/file.h
        headers/apfs/internal/extent.h
        headers/apfs/internal/directory.h
        headers/apfs/internal/xattr.h
        headers/apfs/internal/object.h
        headers/apfs/internal/object_cache.h
        headers/apfs/internal/container_view.h
        headers/apfs/internal/base.h
        headers/apfs/session.h
        headers/apfs/object.h
        headers/apfs/volume.h
        headers/apfs/base.h
        DESTINATION include/apfs)
install(TARGETS apfs_static apfs_shared
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
