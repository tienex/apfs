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

project(nxmount)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

include_directories(${CMAKE_SOURCE_DIR}/libapfs/headers
                    ${CMAKE_SOURCE_DIR}/libnx/headers
                    ${CMAKE_SOURCE_DIR}/libnxtools/headers
                    ${CMAKE_SOURCE_DIR}/libnxcompat/headers
                    ${CMAKE_BINARY_DIR}/libnxcompat/headers)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(FUSE)

if (NOT FUSE_FOUND)
    message(STATUS "FUSE not found, won't build mount_apfs")
    return()
endif ()

include_directories(${FUSE_INCLUDE_DIRS})
link_directories(${FUSE_LIBRARY_DIRS})

set(CMAKE_CXX_STANDARD 11)

set(FUSE_PATCHED_NOVOLICON TRUE)
if (APPLE AND FUSE_PATCHED_NOVOLICON)
    list(APPEND FUSE_DEFINITIONS FUSE_HAS_NOVOLICON)
endif ()

add_library(apfs_fuse STATIC
            fuse_bridge.cpp
            base.cpp
            object.cpp
            file.cpp
            directory.cpp
            rsrcfork.cpp
            volume.cpp
            nx_root.cpp
            nx_volume.cpp
            xattr_directory.cpp
            xattr_object_directory.cpp
            xattr_file.cpp
            )
set_target_properties(apfs_fuse PROPERTIES COMPILE_DEFINITIONS "${FUSE_DEFINITIONS};FUSE_USE_VERSION=29")
target_link_libraries(apfs_fuse apfs_shared nxtools ${FUSE_LIBRARIES})

add_executable(mount_apfs mount_apfs.cpp)
set_target_properties(mount_apfs PROPERTIES COMPILE_DEFINITIONS "${FUSE_DEFINITIONS};FUSE_USE_VERSION=29")
target_link_libraries(mount_apfs apfs_fuse)

add_executable(mount_nx mount_nx.cpp)
set_target_properties(mount_nx PROPERTIES COMPILE_DEFINITIONS "${FUSE_DEFINITIONS};FUSE_USE_VERSION=29")
target_link_libraries(mount_nx apfs_fuse)

install(TARGETS mount_apfs mount_nx
        RUNTIME DESTINATION sbin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
