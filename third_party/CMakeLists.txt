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

project(nxapfs_third_party)

cmake_minimum_required(VERSION 3.5)

include(CheckIncludeFile)
include(CheckLibraryExists)

#
# Build internal libnx_xo only if libxo is not supported.
#
CHECK_INCLUDE_FILE(libxo/xo.h HAVE_LIBXO_XO_H)
if (HAVE_LIBXO_XO_H)
    set(INCLUDE_LIBXO_XO_H TRUE PARENT_SCOPE)
else ()
    CHECK_INCLUDE_FILE(xo.h HAVE_XO_H)
endif ()
if (HAVE_LIBXO_XO_H OR HAVE_XO_H)
    CHECK_LIBRARY_EXISTS(xo xo_emit "" HAVE_LIBXO)
endif ()

if (NOT HAVE_LIBXO)
    add_subdirectory(libxo)
    set(LIBXO_LIBRARY nx_xo PARENT_SCOPE)
else ()
    set(LIBXO_LIBRARY xo PARENT_SCOPE)
endif ()

#
# libnx_table is always built.
#
add_subdirectory(libtable)
