# Set directories
set(HEDGELIB_INCLUDE_DIR "include")
set(HEDGELIB_SOURCE_DIR "src")

# Set includes
set(HEDGELIB_INCLUDES
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/archives/hl_archive.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/archives/hl_hh_archive.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/archives/hl_pacx.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/effects/hl_fxcol.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/effects/hl_grif.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hh/hl_hh_light.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hh/hl_hh_needle_texture_streaming.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hh/hl_hh_needle.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_bina.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_file.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_hh_mirage.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_mem_stream.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_path.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_stream.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/materials/hl_hh_material.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/models/hl_hh_model.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/shader/hl_hh_shader.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/terrain/hl_hh_terrain.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/textures/hl_hh_texture.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_array.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_blob.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_compression.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_internal.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_languages_autogen.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_math.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_memory.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_ordered_map.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_radix_tree.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_resource.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_scene.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_tables.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_text.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_tool_helpers.h"
)

# Set sources
set(HEDGELIB_SOURCES
    "${HEDGELIB_SOURCE_DIR}/archives/hl_archive.cpp"
    "${HEDGELIB_SOURCE_DIR}/archives/hl_hh_archive.cpp"
    "${HEDGELIB_SOURCE_DIR}/archives/hl_in_pacx_type_autogen.h"
    "${HEDGELIB_SOURCE_DIR}/archives/hl_pacx.cpp"
    "${HEDGELIB_SOURCE_DIR}/effects/hl_grif.cpp"
    "${HEDGELIB_SOURCE_DIR}/hh/hl_hh_light.cpp"
    "${HEDGELIB_SOURCE_DIR}/hh/hl_hh_needle_texture_streaming.cpp"
    "${HEDGELIB_SOURCE_DIR}/hh/hl_hh_needle.cpp"
    "${HEDGELIB_SOURCE_DIR}/io/hl_bina.cpp"
    "${HEDGELIB_SOURCE_DIR}/io/hl_file.cpp"
    "${HEDGELIB_SOURCE_DIR}/io/hl_hh_mirage.cpp"
    "${HEDGELIB_SOURCE_DIR}/io/hl_mem_stream.cpp"
    "${HEDGELIB_SOURCE_DIR}/io/hl_path.cpp"
    "${HEDGELIB_SOURCE_DIR}/io/hl_stream.cpp"
    "${HEDGELIB_SOURCE_DIR}/materials/hl_hh_material.cpp"
    "${HEDGELIB_SOURCE_DIR}/models/hl_hh_model.cpp"
    "${HEDGELIB_SOURCE_DIR}/shader/hl_hh_shader.cpp"
    "${HEDGELIB_SOURCE_DIR}/terrain/hl_hh_terrain.cpp"
    "${HEDGELIB_SOURCE_DIR}/textures/hl_hh_texture.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_blob.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_compression.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_in_blob.h"
    "${HEDGELIB_SOURCE_DIR}/hl_in_posix.h"
    "${HEDGELIB_SOURCE_DIR}/hl_in_tool_common_text.h"
    "${HEDGELIB_SOURCE_DIR}/hl_in_win32.h"
    "${HEDGELIB_SOURCE_DIR}/hl_internal.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_math.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_radix_tree.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_resource.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_scene.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_text.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_tool_helpers.cpp"
)

# Setup library
add_library(HedgeLib ${HEDGELIB_INCLUDES} ${HEDGELIB_SOURCES})
add_library(HedgeLib::HedgeLib ALIAS HedgeLib)

set_target_properties(HedgeLib PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    ${HEDGELIB_USE_DEBUG_POSTFIX}
)

# Setup DLL preprocessor definitions
if(BUILD_SHARED_LIBS)
    target_compile_definitions(HedgeLib
        PUBLIC HL_IS_DLL
        PRIVATE HL_IS_BUILDING_DLL
    )
endif()

# Disable intrinsics if requested
if(HEDGELIB_DISABLE_INTRINSICS)
    target_compile_definitions(HedgeLib PUBLIC HL_DISABLE_INTRINSICS)
endif()

# Setup platform-specific stuff
if(WIN32)
    # Force usage of ANSI Win32 API if requested
    if(HEDGELIB_WIN32_FORCE_ANSI)
        target_compile_definitions(HedgeLib PUBLIC HL_WIN32_FORCE_ANSI)
    else()
        target_compile_definitions(HedgeLib
            PRIVATE
                _UNICODE
                UNICODE
        )
    endif()

# Use "libhedge" instead of "libHedgeLib" on Unix-based
# platforms to follow Unix naming conventions
elseif(UNIX)
    set_target_properties(HedgeLib PROPERTIES OUTPUT_NAME "hedge")
endif()

# Find FBX SDK and add it to HedgeLib dependencies
if(NOT TARGET fbx::sdk)
    # TODO: Update FindFbx.cmake to account for arm64!
    find_package(Fbx)
endif()

if(TARGET fbx::sdk)
    set(HEDGELIB_USES_FBX_SDK ON)
    target_compile_definitions(HedgeLib PRIVATE HL_USE_FBX_SDK)
    list(APPEND HEDGELIB_PRIVATE_DEPEND_LIBS fbx::sdk)
endif()

# Find robin_hood and add it to HedgeLib dependencies
if(NOT TARGET robin_hood::robin_hood)
    message(STATUS "Searching for robin_hood...")
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
    find_package(robin_hood QUIET REQUIRED)
    unset(CMAKE_FIND_PACKAGE_PREFER_CONFIG)
endif()

list(APPEND HEDGELIB_PUBLIC_DEPEND_LIBS robin_hood::robin_hood)

# Find glm and add it to HedgeLib dependencies
if(NOT TARGET glm::glm)
    message(STATUS "Searching for glm...")
    find_package(glm QUIET REQUIRED)
    if(TARGET glm AND NOT TARGET glm::glm)
        add_library(glm::glm ALIAS glm)
    endif()
endif()

list(APPEND HEDGELIB_PRIVATE_DEPEND_LIBS glm::glm)

# TODO: Do we actually need this workaround, or did I just misunderstand something?

## Work around bug on macOS that causes clang to attempt to link to
## glm despite it having no actual compiled binaries to link to.
#if(APPLE)
    #list(APPEND HEDGELIB_PRIVATE_INCLUDE_DIRS ${GLM_INCLUDE_DIRS})
#else()
    #list(APPEND HEDGELIB_PRIVATE_DEPEND_LIBS glm)
#endif()

# Download DirectXMath
message(STATUS "Downloading and/or configuring DirectXMath as necessary...")
FetchContent_Declare(DirectXMath
    GIT_REPOSITORY https://github.com/microsoft/DirectXMath.git
    GIT_TAG origin/main
    GIT_SHALLOW TRUE
)

# Setup DirectXMath and add it to HedgeLib private includes
FetchContent_Populate(DirectXMath)
list(APPEND HEDGELIB_PRIVATE_INCLUDE_DIRS ${directxmath_SOURCE_DIR}/Inc)

# Download POSIX sal.h for usage with DirectXMath on non-Windows platforms
if(NOT WIN32)
    file(DOWNLOAD
        https://raw.githubusercontent.com/dotnet/runtimelab/feature/NativeAOT/src/coreclr/pal/inc/rt/sal.h
        ${directxmath_SOURCE_DIR}/Inc/sal.h
    )
endif()

# Find LZ4 and add it to HedgeLib dependencies
if(NOT TARGET LZ4::LZ4)
    message(STATUS "Searching for lz4...")
    find_package(LZ4 QUIET REQUIRED)
endif()

list(APPEND HEDGELIB_PRIVATE_DEPEND_LIBS LZ4::LZ4)

# Find ZLIB and add it to HedgeLib dependencies
if(NOT TARGET ZLIB::ZLIB)
    message(STATUS "Searching for zlib...")
    find_package(ZLIB QUIET REQUIRED)
endif()

list(APPEND HEDGELIB_PRIVATE_DEPEND_LIBS ZLIB::ZLIB)

# Setup dependencies
target_include_directories(HedgeLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${HEDGELIB_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${HEDGELIB_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(HedgeLib
    PUBLIC ${HEDGELIB_PUBLIC_DEPEND_LIBS}
    PRIVATE ${HEDGELIB_PRIVATE_DEPEND_LIBS}
)

# Install include files
install(DIRECTORY ${HEDGELIB_INCLUDE_DIR}/hedgelib
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Setup CMake package config files
install(TARGETS HedgeLib
    EXPORT HedgeLibTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(HEDGELIB_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/HedgeLib)

install(EXPORT HedgeLibTargets
    FILE HedgeLibTargets.cmake
    NAMESPACE HedgeLib::
    DESTINATION ${HEDGELIB_INSTALL_CONFIGDIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeLibConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/HedgeLibConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeLibConfig.cmake
    INSTALL_DESTINATION ${HEDGELIB_INSTALL_CONFIGDIR}
)

set(HEDGELIB_INSTALL_CONFIG_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeLibConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeLibConfigVersion.cmake
)

if(NOT BUILD_SHARED_LIBS)
    if(HEDGELIB_USES_FBX_SDK)
        list(APPEND HEDGELIB_INSTALL_CONFIG_FILES
            ${PROJECT_SOURCE_DIR}/cmake/modules/FindFbx.cmake
        )
    endif()

    list(APPEND HEDGELIB_INSTALL_CONFIG_FILES
        ${PROJECT_SOURCE_DIR}/cmake/modules/FindLZ4.cmake
    )

    list(APPEND HEDGELIB_INSTALL_CONFIG_FILES
        ${PROJECT_SOURCE_DIR}/cmake/modules/Findrobin_hood.cmake
    )
endif()

install(FILES
    ${HEDGELIB_INSTALL_CONFIG_FILES}
    DESTINATION ${HEDGELIB_INSTALL_CONFIGDIR}
)

# Export HedgeLib
if(NOT BUILD_SHARED_LIBS)
    if(HEDGELIB_USES_FBX_SDK)
        configure_file(
            ${PROJECT_SOURCE_DIR}/cmake/modules/FindFbx.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/FindFbx.cmake
            COPYONLY
        )
    endif()

    configure_file(
        ${PROJECT_SOURCE_DIR}/cmake/modules/FindLZ4.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/FindLZ4.cmake
        COPYONLY
    )
endif()

export(EXPORT HedgeLibTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/HedgeLibTargets.cmake
    NAMESPACE HedgeLib::
)

export(PACKAGE HedgeLib)
