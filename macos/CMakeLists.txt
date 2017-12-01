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

project(macos_nxapfs)

if (WIN32)
    return()
endif ()

if (APPLE)
    set(LIBNX_NAME "libnx.1.0.0.dylib")
    set(LIBAPFS_NAME "libapfs.1.0.0.dylib")

    find_program(INSTALL_NAME_TOOL "install_name_tool")
    if (NOT INSTALL_NAME_TOOL)
        return()
    endif ()

    set(MOUNT_CHANGE_RPATH_COMMAND ${INSTALL_NAME_TOOL} -change "@rpath/${LIBNX_NAME}" "@executable_path/../Libraries/${LIBNX_NAME}"
                                                        -change "@rpath/${LIBAPFS_NAME}" "@executable_path/../Libraries/${LIBAPFS_NAME}"
                                                        "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Resources/mount_nx")
    set(PROBE_CHANGE_RPATH_COMMAND ${INSTALL_NAME_TOOL} -change "@rpath/${LIBNX_NAME}" "@executable_path/../Libraries/${LIBNX_NAME}"
                                                        -change "@rpath/${LIBAPFS_NAME}" "@executable_path/../Libraries/${LIBAPFS_NAME}"
                                                        "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Resources/nx_probe")
    set(LIBNX_CHANGE_RPATH_COMMAND ${INSTALL_NAME_TOOL} -id "@executable_path/../Libraries/${LIBNX_NAME}"
                                                        "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Libraries/${LIBNX_NAME}")
    set(LIBAPFS_CHANGE_RPATH_COMMAND ${INSTALL_NAME_TOOL} -id "@executable_path/../Libraries/${LIBAPFS_NAME}"
                                                          -change "@rpath/${LIBNX_NAME}" "@executable_path/../Libraries/${LIBNX_NAME}"
                                                          "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Libraries/${LIBAPFS_NAME}")
else ()
    set(LIBNX_NAME "libnx.so.1.0.0")
    set(LIBAPFS_NAME "libapfs.so.1.0.0")

    find_program(CHRPATH "chrpath")
    if (NOT CHRPATH)
        return()
    endif ()
    set(MOUNT_CHANGE_RPATH_COMMAND ${CHRPATH} -r $$"ORIGIN/../Libraries" "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Resources/mount_nx")
    set(PROBE_CHANGE_RPATH_COMMAND ${CHRPATH} -r $$"ORIGIN/../Libraries" "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Resources/nx_probe")
    set(LIBNX_CHANGE_RPATH_COMMAND :)
    set(LIBAPFS_CHANGE_RPATH_COMMAND :)
endif ()

#
# This creates an nxapfs.fs bundle for legacy macOS.
#
add_custom_target(nxapfs_fs ALL
                  COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs"
                  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/nxapfs.fs" "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs"
                  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Resources"
                  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Libraries"
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libnx/${LIBNX_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Libraries"
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libapfs/${LIBAPFS_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Libraries"
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/mount/mount_nx" "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Resources"
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/tools/nx_probe" "${CMAKE_CURRENT_BINARY_DIR}/nxapfs.fs/Contents/Resources"
                  COMMAND ${LIBNX_CHANGE_RPATH_COMMAND}
                  COMMAND ${LIBAPFS_CHANGE_RPATH_COMMAND}
                  COMMAND ${MOUNT_CHANGE_RPATH_COMMAND}
                  COMMAND ${PROBE_CHANGE_RPATH_COMMAND}
                  DEPENDS mount_nx nx_probe
                  COMMENT "Building macOS filesystem bundle")
