cmake_minimum_required(VERSION 3.12)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(FetchContent)

# Setup project
project(HedgeLib)

# Set module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

# Options
option(HEDGELIB_FORCE_STATIC_LINK_DEPS "Force CMake to always statically link HedgeLib's dependencies, even on Unix-like platforms (not recommended)" OFF)
option(HEDGELIB_DISABLE_INTRINSICS "Disable all usage of intrinsics throughout HedgeLib and the HedgeTools (not recommended)" OFF)

# Windows-specific Options
if(WIN32)
    option(HEDGELIB_WIN32_FORCE_ANSI "Force HedgeLib to use the ANSI Win32 API instead of wide-char UNICODE Win32 functions (not recommended)" OFF)
endif()

# Disable annoying MSVC unsafe string warnings
if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Add HedgeLib
add_subdirectory(HedgeLib)

# Don't add HedgeTools if this CMakeLists.txt file was included from another CMakeLists.txt file
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    #add_subdirectory(HedgeRender)
    #add_subdirectory(HedgeTest)
    add_subdirectory(HedgeTools)
endif()
