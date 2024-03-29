# Setup CMake
cmake_minimum_required(VERSION 3.18...3.22)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
    set(CMAKE_DEBUG_POSTFIX d)
    set(HEDGELIB_USE_DEBUG_POSTFIX
        DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    )
endif()

# Setup project
project(HedgeLib
    VERSION 1.0.0
    LANGUAGES CXX
)

# Include dependencies
include(FetchContent)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Set module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

# Options
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    set(HEDGELIB_ROOT_CMAKE_FILE ON)
else()
    set(HEDGELIB_ROOT_CMAKE_FILE OFF)
endif()

option(HEDGELIB_BUILD_HEDGERENDER
    "Build the HedgeRender libraries (NOTE: this is done regardless of this option if HEDGELIB_BUILD_HEDGETOOLS is set)"
    ${HEDGELIB_ROOT_CMAKE_FILE}
)

option(HEDGELIB_BUILD_HEDGETOOLS
    "Build the HedgeTools"
    ${HEDGELIB_ROOT_CMAKE_FILE}
)

option(BUILD_SHARED_LIBS
    "Build HedgeLib as shared libraries instead of static"
    OFF
)

option(HEDGERENDER_USE_IMGUI
    "Build HedgeRender with Imgui (NOTE: this is required to build HedgeEdit)"
    ON
)

option(HEDGELIB_DISABLE_INTRINSICS
    "Disable all usage of intrinsics throughout HedgeLib and the HedgeTools (not recommended)"
    OFF
)

set(HEDGELIB_LOCAL_DEPENDENCIES_DIR
    "${PROJECT_SOURCE_DIR}/Dependencies/${CMAKE_GENERATOR_PLATFORM}"
    CACHE PATH
    "The path that CMake will first check when searching for dependencies required to build HedgeLib"
)

# Platform-specific options
if(WIN32)
    option(HEDGELIB_WIN32_FORCE_ANSI
        "Force HedgeLib to use the ANSI Win32 API instead of wide-char UNICODE Win32 functions (not recommended)"
        OFF
    )
endif()

if(MSVC)
    # Disable annoying unsafe string warnings
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

    # Enable strict conformance mode and UTF-8 charset
    add_compile_options(/permissive- /utf-8)
endif()

# Make CMake check for packages in "Dependencies" subdirectory
list(APPEND CMAKE_PREFIX_PATH
    ${HEDGELIB_LOCAL_DEPENDENCIES_DIR}
)

# Make CMake output all libraries and runtimes to top-level binary directory
# (This is so .dlls will be in same folder as .exes on Windows if building shared libraries)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Build HedgeLib
add_subdirectory(HedgeLib)

# Build HedgeRender if requested or required
if(HEDGELIB_BUILD_HEDGERENDER OR HEDGELIB_BUILD_HEDGETOOLS)
    add_subdirectory(HedgeRender)
endif()

# Build HedgeTools if requested
if(HEDGELIB_BUILD_HEDGETOOLS)
    add_subdirectory(HedgeTools)
endif()

# Install license file
install(FILES
    ${PROJECT_SOURCE_DIR}/LICENSE.txt
    ${PROJECT_SOURCE_DIR}/THIRD-PARTY-LICENSES.txt
    DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/hedgelib
)
