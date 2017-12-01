# This module can find FUSE Library
#
# Requirements:
# - CMake >= 3.0
#
# The following variables will be defined for your use:
# - FUSE_FOUND : was FUSE found?
# - FUSE_INCLUDE_DIRS : FUSE include directory
# - FUSE_LIBRARIES : FUSE library
# - FUSE_CFLAGS : FUSE cflags
# - FUSE_VERSION : complete version of FUSE (major.minor)
# - FUSE_MAJOR_VERSION : major version of FUSE
# - FUSE_MINOR_VERSION : minor version of FUSE
#
# Example Usage:
#
# 1. Copy this file in the root of your project source directory
# 2. Then, tell CMake to search this non-standard module in your project directory by adding to your CMakeLists.txt:
# set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
# 3. Finally call find_package() once, here are some examples to pick from
#
# Require FUSE 2.6 or later
# find_package(FUSE 2.6 REQUIRED)
#
# if(FUSE_FOUND)
# add_definitions(${FUSE_CFLAGS})
# include_directories(${FUSE_INCLUDE_DIRS})
# add_executable(myapp myapp.c)
# target_link_libraries(myapp ${FUSE_LIBRARIES})
# endif()

#=============================================================================
# Copyright (c) 2017, Orlando Bassotto
# Copyright (c) 2012, julp
#
# Distributed under the OSI-approved BSD License
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#=============================================================================

cmake_minimum_required(VERSION 3.0)

set(FUSE_DEBUG FALSE)

include(CheckIncludeFile)

########## Private ##########
function(fusedebug _varname)
    if(FUSE_DEBUG)
        message("${_varname} = ${${_varname}}")
    endif(FUSE_DEBUG)
endfunction(fusedebug)

########## Public ##########
set(FUSE_FOUND TRUE)
unset(FUSE_INCLUDE_DIRS)
unset(FUSE_LIBRARIES)
unset(FUSE_CFLAGS)
unset(FUSE_DEFINITIONS)

find_package(PkgConfig QUIET)

unset(PC_FUSE_INCLUDE_DIRS)
unset(PC_FUSE_LIBRARY_DIRS)

if (PKG_CONFIG_FOUND)
    pkg_check_modules(PC_FUSE "fuse" QUIET)
    if(PC_FUSE_FOUND)
        string(STRIP "${PC_FUSE_LIBRARIES}" PC_FUSE_LIBRARIES)
        string(STRIP "${PC_FUSE_LIBRARY_DIRS}" PC_FUSE_LIBRARY_DIRS)
        string(STRIP "${PC_FUSE_LDFLAGS}" PC_FUSE_LDFLAGS)
        string(STRIP "${PC_FUSE_LDFLAGS_OTHER}" PC_FUSE_LDFLAGS_OTHER)
        string(STRIP "${PC_FUSE_INCLUDE_DIRS}" PC_FUSE_INCLUDE_DIRS)
        string(STRIP "${PC_FUSE_CFLAGS}" PC_FUSE_CFLAGS)
        string(STRIP "${PC_FUSE_CFLAGS_OTHER}" PC_FUSE_CFLAGS_OTHER)
        fusedebug(PC_FUSE_LIBRARIES)
        fusedebug(PC_FUSE_LIBRARY_DIRS)
        fusedebug(PC_FUSE_LDFLAGS)
        fusedebug(PC_FUSE_LDFLAGS_OTHER)
        fusedebug(PC_FUSE_INCLUDE_DIRS)
        fusedebug(PC_FUSE_CFLAGS)
        fusedebug(PC_FUSE_CFLAGS_OTHER)
        if (PC_FUSE_CFLAGS)
            list(APPEND FUSE_CFLAGS ${PC_FUSE_CFLAGS})
        endif ()
        if (PC_FUSE_CFLAGS_OTHER)
            list(APPEND FUSE_CFLAGS ${PC_FUSE_CFLAGS_OTHER})
        endif ()
    endif ()
endif ()

if (NOT PC_FUSE_INCLUDE_DIRS)
    set(PC_FUSE_INCLUDE_DIRS
        "/usr/include"
        "/usr/include/fuse"
        "/usr/local/include/fuse"
        "/usr/local/include/osxfuse/fuse"
        "/opt/include/fuse"
        "/opt/include/osxfuse/fuse"
        "/opt/local/include/fuse"
        "/opt/local/include/osxfuse/fuse"
        ${FUSE_INCLUDE_DIR})
endif ()

if (NOT PC_FUSE_LIBRARY_DIRS)
    set(PC_FUSE_LIBRARY_DIRS
        "/usr/lib"
        "/usr/local/lib"
        "/opt/lib"
        "/opt/local/lib"
        ${FUSE_LIBRARY_DIR})
endif ()

if (NOT PC_FUSE_LIBRARIES)
    set(PC_FUSE_LIBRARIES fuse osxfuse perfuse refuse)
endif ()

find_path(FUSE_INCLUDE_DIRS
          NAMES fuse.h
          PATHS ${PC_FUSE_INCLUDE_DIRS}
          DOC "Include directories for FUSE"
          NO_DEFAULT_PATH)

find_library(FUSE_LIBRARIES
             NAMES ${PC_FUSE_LIBRARIES}
             PATHS ${PC_FUSE_LIBRARY_DIRS}
             DOC "Libraries for FUSE"
             NO_DEFAULT_PATH)

if (NOT FUSE_LIBRARIES)
    # On Linux it may be onto a multiarch dir, try to link.
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(OLD_CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES}")
        set(OLD_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
        set(OLD_CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
        foreach (_fuse_library ${PC_FUSE_LIBRARIES})
            set(CMAKE_REQUIRED_INCLUDES "${OLD_CMAKE_REQUIRED_INCLUDES}" "${FUSE_INCLUDE_DIRS}")
            set(CMAKE_REQUIRED_LIBRARIES "${OLD_CMAKE_REQUIRED_LIBRARIES}" "${_fuse_library}")
            set(CMAKE_REQUIRED_DEFINITIONS "${OLD_CMAKE_REQUIRED_DEFINITIONS}" "${FUSE_CFLAGS}")
            check_c_source_compiles("extern int fuse_main(); int main() { return fuse_main(); }" FUSE_LIBRARY_CHECK_${_fuse_library})
            if (${FUSE_LIBRARY_CHECK_${_fuse_library}})
                set(FUSE_LIBRARIES ${_fuse_library})
                break ()
            endif ()
        endforeach ()
        set(CMAKE_REQUIRED_INCLUDES "${OLD_CMAKE_REQUIRED_INCLUDES}")
        set(CMAKE_REQUIRED_LIBRARIES "${OLD_CMAKE_REQUIRED_LIBRARIES}")
        set(CMAKE_REQUIRED_DEFINITIONS "${OLD_CMAKE_REQUIRED_DEFINITIONS}")
    endif ()
elseif (FUSE_LIBRARIES MATCHES "refuse")
    find_library(PUFFS_LIBRARY
                 NAMES puffs
                 PATHS ${PC_FUSE_LIBRARY_DIRS}
                 DOC "Libraries for PUFFS"
                 NO_DEFAULT_PATH)
    if (PUFFS_LIBRARY)
        list(APPEND FUSE_LIBRARIES ${PUFFS_LIBRARY})
    endif ()
endif ()

if (NOT FUSE_LIBRARIES)
    set(FUSE_FOUND FALSE)
else ()
    list(APPEND FUSE_CFLAGS "-D_FILE_OFFSET_BITS=64")
endif ()

if (FUSE_FOUND)
    unset(_fuse_common_path)
    if (EXISTS "${FUSE_INCLUDE_DIRS}/fuse/fuse_common.h")
        set(_fuse_common_path "${FUSE_INCLUDE_DIRS}/fuse/fuse_common.h")
    elseif (EXISTS "${FUSE_INCLUDE_DIRS}/fuse_common.h")
        set(_fuse_common_path "${FUSE_INCLUDE_DIRS}/fuse_common.h")
    else ()
        set(_fuse_common_path "${FUSE_INCLUDE_DIRS}/fuse.h")
    endif()
    if (_fuse_common_path)
        file(READ ${_fuse_common_path} _contents)
        string(REGEX REPLACE ".*#define[ \t]+FUSE_MAJOR_VERSION[ \t]+([0-9]+).*" "\\1" FUSE_MAJOR_VERSION "${_contents}")
        string(REGEX REPLACE ".*#define[ \t]+FUSE_MINOR_VERSION[ \t]+([0-9]+).*" "\\1" FUSE_MINOR_VERSION "${_contents}")
        set(FUSE_VERSION "${FUSE_MAJOR_VERSION}.${FUSE_MINOR_VERSION}")
        unset(_contents)
        unset(_fuse_common_path)
   endif ()

    include(CheckCSourceCompiles)
    # Backup CMAKE_REQUIRED_*
    set(OLD_CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES}")
    set(OLD_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
    set(OLD_CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
    # Add FUSE compilation flags
    set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES}" "${FUSE_INCLUDE_DIRS}")
    set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}" "${FUSE_LIBRARIES}")
    set(CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}" "${FUSE_CFLAGS}")
    check_c_source_compiles("#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

int main(void) {
return 0;
}" FUSE_CFLAGS_CHECK)
    # Restore CMAKE_REQUIRED_*
    set(CMAKE_REQUIRED_INCLUDES "${OLD_CMAKE_REQUIRED_INCLUDES}")
    set(CMAKE_REQUIRED_LIBRARIES "${OLD_CMAKE_REQUIRED_LIBRARIES}")
    set(CMAKE_REQUIRED_DEFINITIONS "${OLD_CMAKE_REQUIRED_DEFINITIONS}")
    # Convert FUSE_CFLAGS to FUSE_DEFINITIONS
    list(REMOVE_DUPLICATES FUSE_CFLAGS)
    string(REPLACE "-D" "" FUSE_DEFINITIONS "${FUSE_CFLAGS}")
    string(REPLACE ";" " " _fuse_cflags_str "${FUSE_CFLAGS}")
    list(FILTER FUSE_DEFINITIONS EXCLUDE REGEX "^-.*")
    list(REMOVE_DUPLICATES FUSE_DEFINITIONS)
    set(FUSE_CFLAGS ${_fuse_cflags_str})
    unset(_fuse_cflags_str)
endif ()

if (FUSE_INCLUDE_DIRS)
    include(FindPackageHandleStandardArgs)
    if (FUSE_FIND_REQUIRED AND NOT FUSE_FIND_QUIETLY)
        find_package_handle_standard_args(FUSE REQUIRED_VARS FUSE_LIBRARIES FUSE_INCLUDE_DIRS VERSION_VAR FUSE_VERSION)
    else ()
        find_package_handle_standard_args(FUSE "FUSE not found" FUSE_LIBRARIES FUSE_INCLUDE_DIRS)
    endif ()
else ()
    if (FUSE_FIND_REQUIRED AND NOT FUSE_FIND_QUIETLY)
        message(FATAL_ERROR "Could not find FUSE include directory")
    endif ()
endif ()

mark_as_advanced(
    FUSE_INCLUDE_DIRS
    FUSE_LIBRARIES
)

unset(PC_FUSE_LIBRARIES)
unset(PC_FUSE_LIBRARY_DIRS)
unset(PC_FUSE_LDFLAGS)
unset(PC_FUSE_LDFLAGS_OTHER)
unset(PC_FUSE_INCLUDE_DIRS)
unset(PC_FUSE_CFLAGS)
unset(PC_FUSE_CFLAGS_OTHER)

# IN (args)
fusedebug("FUSE_FIND_COMPONENTS")
fusedebug("FUSE_FIND_REQUIRED")
fusedebug("FUSE_FIND_QUIETLY")
fusedebug("FUSE_FIND_VERSION")
# OUT
# Found
fusedebug("FUSE_FOUND")
# Definitions
fusedebug("FUSE_CFLAGS")
fusedebug("FUSE_DEFINITIONS")
# Linking
fusedebug("FUSE_INCLUDE_DIRS")
fusedebug("FUSE_LIBRARIES")
# Version
fusedebug("FUSE_MAJOR_VERSION")
fusedebug("FUSE_MINOR_VERSION")
fusedebug("FUSE_VERSION")
