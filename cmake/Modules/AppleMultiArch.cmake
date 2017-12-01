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

include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)

#
# Determine the architectures supported by macOS version.
#
if (APPLE)
    exec_program(sw_vers ARGS -productVersion OUTPUT_VARIABLE MACOS_VERSION)
    if (MACOS_VERSION MATCHES "^10\\.[0123]\\.")
        set(_APPLE_ARCHITECTURES ppc)
    elseif (MACOS_VERSION MATCHES "^10\\.4\\.")
        set(_APPLE_ARCHITECTURES i386;ppc;ppc64)
    elseif (MACOS_VERSION MATCHES "^10\\.5\\.")
        set(_APPLE_ARCHITECTURES i386;x86_64;ppc;ppc64)
    else ()
        set(_APPLE_ARCHITECTURES x86_64;i386)
    endif ()

    #
    # Try to compile with each arch.
    #
    set(CMAKE_OSX_ARCHITECTURES )
    foreach(_APPLE_ARCHITECTURE ${_APPLE_ARCHITECTURES})
        set(CMAKE_REQUIRED_FLAGS "-arch ${_APPLE_ARCHITECTURE}")
        CHECK_C_SOURCE_COMPILES("int main() { return 0; }" _APPLE_C_${_APPLE_ARCHITECTURE})
        CHECK_CXX_SOURCE_COMPILES("int main() { return 0; }" _APPLE_CXX_${_APPLE_ARCHITECTURE})
        if (${_APPLE_C_${_APPLE_ARCHITECTURE}} AND ${_APPLE_CXX_${_APPLE_ARCHITECTURE}})
            message(STATUS "Architecture ${_APPLE_ARCHITECTURE} supported")
            list(APPEND CMAKE_OSX_ARCHITECTURES ${_APPLE_ARCHITECTURE})
        endif ()
        unset(CMAKE_REQUIRED_FLAGS)
    endforeach()
endif ()

