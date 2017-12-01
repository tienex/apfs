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

project(nxtools)

cmake_minimum_required(VERSION 3.5)

include_directories(${CMAKE_SOURCE_DIR}/libnx/headers
                    ${CMAKE_SOURCE_DIR}/libapfs/headers
                    ${CMAKE_SOURCE_DIR}/libnxtools/headers
                    ${CMAKE_SOURCE_DIR}/libnxcompat/headers
                    ${CMAKE_BINARY_DIR}/libnxcompat/headers)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 11)

add_executable(apfs_probe apfs_probe.cpp)
target_link_libraries(apfs_probe nx_shared)

add_executable(nx_probe nx_probe.cpp)
target_link_libraries(nx_probe nx_shared)

add_executable(nx_tool
               main.cpp
               nx_scavenge.cpp
               nx_omap.cpp
               apfs_omap.cpp
               apfs_traverse.cpp
               apfs_content.cpp
               apfs_extract.cpp)
target_link_libraries(nx_tool nx_shared apfs_shared nxtools)

add_custom_target(nx_scavenge ALL COMMAND ${CMAKE_COMMAND} -E create_symlink nx_tool nx_scavenge)
add_dependencies(nx_scavenge nx_tool)

add_custom_target(nx_omap ALL COMMAND ${CMAKE_COMMAND} -E create_symlink nx_tool nx_omap)
add_dependencies(nx_omap nx_tool)

add_custom_target(apfs_omap ALL COMMAND ${CMAKE_COMMAND} -E create_symlink nx_tool apfs_omap)
add_dependencies(apfs_omap nx_tool)

add_custom_target(apfs_traverse ALL COMMAND ${CMAKE_COMMAND} -E create_symlink nx_tool apfs_traverse)
add_dependencies(apfs_traverse nx_tool)

add_custom_target(apfs_content ALL COMMAND ${CMAKE_COMMAND} -E create_symlink nx_tool apfs_content)
add_dependencies(apfs_content nx_tool)

add_custom_target(apfs_extract ALL COMMAND ${CMAKE_COMMAND} -E create_symlink nx_tool apfs_extract)
add_dependencies(apfs_extract nx_tool)

install(TARGETS nx_tool
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/nx_scavenge
        ${CMAKE_CURRENT_BINARY_DIR}/nx_omap
        ${CMAKE_CURRENT_BINARY_DIR}/apfs_omap
        ${CMAKE_CURRENT_BINARY_DIR}/apfs_traverse
        ${CMAKE_CURRENT_BINARY_DIR}/apfs_content
        ${CMAKE_CURRENT_BINARY_DIR}/apfs_extract
        DESTINATION bin)
