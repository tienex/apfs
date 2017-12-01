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

project(libnx)

cmake_minimum_required(VERSION 3.5)

if (APPLE AND POLICY CMP0068)
    cmake_policy(SET CMP0068 NEW)
endif ()

if (CMAKE_BUILD_TYPE MATCHES "Release" AND POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)

    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED)
endif ()

include_directories(headers headers/private
                    ${CMAKE_SOURCE_DIR}/libnxcompat/headers
                    ${CMAKE_BINARY_DIR}/libnxcompat/headers)

include_directories(${CMAKE_SOURCE_DIR}/third_party/libtable)
if (NOT HAVE_LIBXO)
    include_directories(${CMAKE_SOURCE_DIR}/third_party/libxo/libxo/libxo)
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 11)

set(libnx_SOURCES
    sources/btree_traverser.cpp
    sources/container.cpp
    sources/context.cpp
    sources/device.cpp
    sources/enumerator.cpp
    sources/object.cpp
    sources/volume.cpp
    sources/format/nx_dumper.c
    sources/format/nx.c
    sources/format/nxdump.c
    sources/format/apfs.c
    sources/format/apfsdump.c)

add_library(nx_static STATIC ${libnx_SOURCES})
target_link_libraries(nx_static nxcompat ${LIBXO_LIBRARY} nx_table)
set_target_properties(nx_static PROPERTIES OUTPUT_NAME "nx")
if (INCLUDE_LIBXO_XO_H)
    set_target_properties(nx_static PROPERTIES COMPILE_DEFINITIONS "HAVE_LIBXO_XO_H")
endif ()

add_library(nx_shared SHARED ${libnx_SOURCES})
target_link_libraries(nx_shared nxcompat ${LIBXO_LIBRARY} nx_table)
set_target_properties(nx_shared PROPERTIES OUTPUT_NAME "nx")
set_target_properties(nx_shared PROPERTIES VERSION ${NXAPFS_VERSION})
if (INCLUDE_LIBXO_XO_H)
    set_target_properties(nx_shared PROPERTIES COMPILE_DEFINITIONS "HAVE_LIBXO_XO_H")
endif ()
if (IPO_SUPPORTED)
    set_target_properties(nx_shared PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

install(FILES
        headers/nx/base.h
        headers/nx/container.h
        headers/nx/context.h
        headers/nx/device.h
        headers/nx/enumerator.h
        headers/nx/logger.h
        headers/nx/nx.h
        headers/nx/object.h
        headers/nx/severity.h
        headers/nx/stack.h
        headers/nx/swap.h
        headers/nx/volume.h
        DESTINATION include/nx)
install(FILES
        headers/nx/format/base.h
        headers/nx/format/apfs.h
        headers/nx/format/nx.h
        DESTINATION include/nx/format)
install(TARGETS nx_static nx_shared
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
