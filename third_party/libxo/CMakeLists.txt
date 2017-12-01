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

project(libnx_xo)

cmake_minimum_required(VERSION 3.5)

include(CheckCSourceCompiles)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckStructHasMember)
include(CheckSymbolExists)

CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILE(stdio_ext.h HAVE_STDIO_EXT_H)
CHECK_INCLUDE_FILE(libutil.h HAVE_LIBUTIL_H)
CHECK_INCLUDE_FILE(sys/un.h HAVE_SYS_UN_H)
CHECK_INCLUDE_FILE(sys/syslog.h HAVE_SYS_SYSLOG_H)

CHECK_FUNCTION_EXISTS(__flbf HAVE___FLBF)

if (CMAKE_SYSTEM_NAME MATCHES "SunOS")
    include_directories(fake-includes)
endif ()

if (HAVE_LIBUTIL_H)
    CHECK_LIBRARY_EXISTS(opendev util "" HAVE_LIBUTIL)
    if (HAVE_LIBUTIL)
        set(CMAKE_REQUIRED_LIBRARIES "util")
        CHECK_FUNCTION_EXISTS(humanize_number HAVE_HUMANIZE_NUMBER)
        unset(CMAKE_REQUIRED_LIBRARIES)
    endif ()
endif ()

if (HAVE_DLFCN_H)
    CHECK_LIBRARY_EXISTS(dl dlopen "" HAVE_LIBDL)
    if (HAVE_LIBDL)
        set(CMAKE_REQUIRED_LIBRARIES "dl")
        CHECK_FUNCTION_EXISTS(dlfunc HAVE_DLFUNC)
        unset(CMAKE_REQUIRED_LIBRARIES)
    endif ()
endif ()

if (HAVE_SYS_SYSLOG_H)
    CHECK_FUNCTION_EXISTS(syslog HAVE_SYSLOG)
endif ()

CHECK_SYMBOL_EXISTS(__isthreaded "stdio.h" HAVE___ISTHREADED)

if (HAVE_SYS_UN_H)
    CHECK_STRUCT_HAS_MEMBER("struct sockaddr_un" "sun_len" "sys/un.h" HAVE_SUN_LEN LANGUAGE C)
endif ()

CHECK_C_SOURCE_COMPILES("__thread int tls; int main(void) { return 0; }" HAVE_THREAD_LOCAL_before)
CHECK_C_SOURCE_COMPILES("int __thread tls; int main(void) { return 0; }" HAVE_THREAD_LOCAL_after)
#CHECK_C_SOURCE_COMPILES("__declspec(thread) tls; int main(void) { return 0; }" HAVE_THREAD_LOCAL_declspec)

if (HAVE_THREAD_LOCAL_before)
    set(HAVE_THREAD_LOCAL THREAD_LOCAL_before)
elseif (HAVE_THREAD_LOCAL_after)
    set(HAVE_THREAD_LOCAL THREAD_LOCAL_after)
elseif (HAVE_THREAD_LOCAL_declspec)
    set(HAVE_THREAD_LOCAL THREAD_LOCAL_declspec)
endif ()

if (HAVE_SYSLOG)
    set(XO_SYSLOG_SOURCE libxo/libxo/xo_syslog.c)
endif ()

configure_file(xo_config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/libxo/xo_config.h)

include_directories(libxo/libxo ${CMAKE_CURRENT_BINARY_DIR}/libxo)

add_library(nx_xo STATIC
            libxo/libxo/libxo.c
            libxo/libxo/xo_encoder.c
            ${XO_SYSLOG_SOURCE})
set_target_properties(nx_xo PROPERTIES
                      COMPILE_DEFINITIONS NO_PRINTFLIKE
                      POSITION_INDEPENDENT_CODE ON
                      C_VISIBILITY_PRESET hidden)
if (HAVE_LIBDL)
    target_link_libraries(nx_xo dl)
endif ()
if (HAVE_LIBUTIL)
    target_link_libraries(nx_xo util)
endif ()

install(TARGETS nx_xo
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
