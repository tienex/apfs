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

project(libnxtools)

cmake_minimum_required(VERSION 3.5)

include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 11)

CHECK_INCLUDE_FILE(syslog.h HAVE_SYSLOG_H)
CHECK_INCLUDE_FILE_CXX(codecvt HAVE_CODECVT)

if (HAVE_CODECVT)
    set(DEFINES HAVE_CODECVT)
endif ()
if (HAVE_SYSLOG_H)
    CHECK_FUNCTION_EXISTS(syslog HAVE_SYSLOG)
    if (HAVE_SYSLOG)
        set(DEFINES ${DEFINES};HAVE_SYSLOG_H;HAVE_SYSLOG)
    endif ()
endif ()

include_directories(headers
                    ${CMAKE_SOURCE_DIR}/libnx/headers
                    ${CMAKE_SOURCE_DIR}/libnxcompat/headers
                    ${CMAKE_BINARY_DIR}/libnxcompat/headers)

add_library(nxtools STATIC
            sources/native.cpp
            sources/string.cpp
            sources/time.cpp
            sources/path.cpp
            sources/file_logger.cpp
            sources/stderr_logger.cpp
            sources/syslog_logger.cpp)
set_target_properties(nxtools PROPERTIES
                      COMPILE_DEFINITIONS "${DEFINES}"
                      POSITION_INDEPENDENT_CODE ON
                      C_VISIBILITY_PRESET hidden)
target_link_libraries(nxtools nxcompat)

install(TARGETS nxtools
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
