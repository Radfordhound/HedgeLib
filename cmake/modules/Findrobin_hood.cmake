# Copyright (c) 2022 Radfordhound
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

# Find robin_hood.h
find_path(ROBIN_HOOD_INCLUDE_DIR robin_hood.h)
mark_as_advanced(ROBIN_HOOD_INCLUDE_DIR)

# Get version number and create library
if(ROBIN_HOOD_INCLUDE_DIR)
    file(READ "${ROBIN_HOOD_INCLUDE_DIR}/robin_hood.h" ROBIN_HOOD_FILE_CONTENT)

    # Find major version number
    if(ROBIN_HOOD_FILE_CONTENT MATCHES "ROBIN_HOOD_VERSION_MAJOR +([0-9]+)")
        set(ROBIN_HOOD_VERSION_MAJOR "${CMAKE_MATCH_1}")
    endif()

    # Find minor version number
    if(ROBIN_HOOD_FILE_CONTENT MATCHES "ROBIN_HOOD_VERSION_MINOR +([0-9]+)")
        set(ROBIN_HOOD_VERSION_MINOR "${CMAKE_MATCH_1}")
    endif()

    # Find patch version number
    if(ROBIN_HOOD_FILE_CONTENT MATCHES "ROBIN_HOOD_VERSION_PATCH +([0-9]+)")
        set(ROBIN_HOOD_VERSION_PATCH "${CMAKE_MATCH_1}")
    endif()

    set(ROBIN_HOOD_VERSION ${ROBIN_HOOD_VERSION_MAJOR}.${ROBIN_HOOD_VERSION_MINOR}.${ROBIN_HOOD_VERSION_PATCH})

    # Create CMake library
    add_library(robin_hood INTERFACE)
    add_library(robin_hood::robin_hood ALIAS robin_hood)

    target_include_directories(robin_hood
        INTERFACE
            ${ROBIN_HOOD_INCLUDE_DIR}
    )

    set(ROBIN_HOOD_LIBRARY robin_hood::robin_hood)
endif()

# Handles find_package arguments and set FBX_FOUND to TRUE if all listed variables and version are valid.
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(robin_hood
  FOUND_VAR ROBIN_HOOD_FOUND
  REQUIRED_VARS ROBIN_HOOD_LIBRARY ROBIN_HOOD_INCLUDE_DIR
  VERSION_VAR ROBIN_HOOD_VERSION)
