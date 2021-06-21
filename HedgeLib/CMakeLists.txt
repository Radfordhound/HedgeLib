# Require at least C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set directories
set(HEDGELIB_INCLUDE_DIR "include")
set(HEDGELIB_SOURCE_DIR "src")

# Setup FBX sdk
find_package(Fbx)

if(FBX_FOUND)
    add_compile_definitions(HL_USE_FBX_SDK)

    list(APPEND HEDGELIB_DEPEND_LIBS_NO_INSTALL ${FBX_LIBRARIES})
    list(APPEND HEDGELIB_DEPEND_INCLUDES ${FBX_INCLUDE_DIRS})
endif()

# Download robin_hood
message(STATUS "Downloading and/or configuring robin_hood as necessary...")
FetchContent_Declare(
    robin_hood
    GIT_REPOSITORY https://github.com/martinus/robin-hood-hashing
    GIT_TAG origin/master
)

# Setup robin_hood
FetchContent_MakeAvailable(robin_hood)
list(APPEND HEDGELIB_DEPEND_LIBS robin_hood)

# Attempt to find glm in system libraries
message(STATUS "Searching for glm...")
find_package(glm CONFIG QUIET)

# If glm could not be found, download it from GitHub
if(NOT TARGET glm::glm AND NOT TARGET glm)
    # Download glm
    message(STATUS "Could not find glm in system paths; falling back to full source compilation.")
    message(STATUS "Downloading and/or configuring glm as necessary...")

    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG origin/master
        GIT_SHALLOW 1
    )

    # Setup glm
    FetchContent_MakeAvailable(glm)

    # Put glm source code into dependencies group
    set_target_properties(uninstall PROPERTIES FOLDER Dependencies)

    # Work around bug on macOS that causes clang to attempt to link to
    # glm despite it having no actual compiled binaries to link to.
    if(APPLE)
        list(APPEND HEDGELIB_DEPEND_INCLUDES ${GLM_INCLUDE_DIRS})
    else()
        list(APPEND HEDGELIB_DEPEND_LIBS glm)
    endif()
else()
    # Work around bug on macOS that causes clang to attempt to link to
    # glm despite it having no actual compiled binaries to link to.
    if(APPLE)
        list(APPEND HEDGELIB_DEPEND_INCLUDES ${GLM_INCLUDE_DIRS})
    else()
        list(APPEND HEDGELIB_DEPEND_LIBS_NO_INSTALL glm)
    endif()
endif()

# Attempt to find lz4 in system libraries
if(NOT HEDGELIB_FORCE_STATIC_LINK_DEPS)
    message(STATUS "Searching for lz4...")
    find_library(HEDGELIB_LZ4_LIB_PATH lz4)
    find_path(HEDGELIB_LZ4_INC_PATH lz4.h)

    if(NOT HEDGELIB_LZ4_LIB_PATH OR NOT HEDGELIB_LZ4_INC_PATH)
        message(STATUS "Could not find lz4 in system paths; falling back to full source compilation.")
    endif()
endif()

# If lz4 could not be found, download it from GitHub
if(NOT HEDGELIB_LZ4_LIB_PATH OR NOT HEDGELIB_LZ4_INC_PATH)
    # Download lz4
    message(STATUS "Downloading and/or configuring lz4 as necessary...")
    FetchContent_Declare(
        lz4
        GIT_REPOSITORY https://github.com/lz4/lz4.git
        GIT_TAG origin/dev
        SOURCE_SUBDIR build/cmake
    )

    # Disable building lz4 executables by default; we only need the library
    option(LZ4_BUILD_CLI "Build lz4 program" OFF)
    option(LZ4_BUILD_LEGACY_LZ4C "Build lz4c progam with legacy argument support" OFF)

    # Setup lz4
    FetchContent_MakeAvailable(lz4)
    list(APPEND HEDGELIB_DEPEND_LIBS lz4_static)
    get_target_property(HEDGELIB_LZ4_INC_PATH lz4_static INCLUDE_DIRECTORIES)

    # Put lz4 source code into dependencies group
    set_target_properties(lz4_static PROPERTIES FOLDER Dependencies)
else()
    list(APPEND HEDGELIB_DEPEND_LIBS_NO_INSTALL ${HEDGELIB_LZ4_LIB_PATH})
endif()

list(APPEND HEDGELIB_DEPEND_INCLUDES ${HEDGELIB_LZ4_INC_PATH})

# Attempt to find zlib in system libraries
if(NOT HEDGELIB_FORCE_STATIC_LINK_DEPS)
    message(STATUS "Searching for zlib...")
    
    # On unix-like systems, "zlib" is named "libz" instead
    if(UNIX)
        set(ZLIB_NAME z)
    else()
        set(ZLIB_NAME zlib)
    endif()

    find_library(HEDGELIB_ZLIB_LIB_PATH ${ZLIB_NAME})
    find_path(HEDGELIB_ZLIB_INC_PATH zlib.h)

    if(NOT HEDGELIB_ZLIB_LIB_PATH OR NOT HEDGELIB_ZLIB_INC_PATH)
        message(STATUS "Could not find zlib in system paths; falling back to full source compilation.")
    endif()
endif()

# If zlib could not be found, download it from GitHub
if(NOT HEDGELIB_ZLIB_LIB_PATH OR NOT HEDGELIB_ZLIB_INC_PATH)
    # Download zlib
    message(STATUS "Downloading and/or configuring zlib as necessary...")
    FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG origin/master
    )

    # Disable installing zlib
    set(SKIP_INSTALL_ALL true)

    # Setup zlib
    FetchContent_MakeAvailable(zlib)
    list(APPEND HEDGELIB_DEPEND_LIBS zlibstatic)
    get_target_property(HEDGELIB_ZLIB_INC_PATH zlibstatic INCLUDE_DIRECTORIES)

    # Put zlib source code into dependencies group
    set_target_properties(zlibstatic PROPERTIES FOLDER Dependencies)
    set_target_properties(zlib PROPERTIES FOLDER Dependencies)
    set_target_properties(example PROPERTIES FOLDER Dependencies)
    set_target_properties(minigzip PROPERTIES FOLDER Dependencies)
else()
    list(APPEND HEDGELIB_DEPEND_LIBS_NO_INSTALL ${HEDGELIB_ZLIB_LIB_PATH})
endif()

list(APPEND HEDGELIB_DEPEND_INCLUDES ${HEDGELIB_ZLIB_INC_PATH})

# Set includes
set(HEDGELIB_INCLUDES
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/archives/hl_archive.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/archives/hl_hh_archive.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/archives/hl_pacx.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/effects/hl_fxcol.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/effects/hl_grif.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_bina.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_file.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_hh_mirage.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_mem_stream.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_path.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/io/hl_stream.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/materials/hl_hh_material.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/models/hl_hh_model.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/shader/hl_hh_shader.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/shader/hl_shader.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/textures/hl_hh_texture.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_blob.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_compression.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_internal.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_languages_autogen.h"
    "${HEDGELIB_INCLUDE_DIR}/hedgelib/hl_math.h"
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
    "${HEDGELIB_SOURCE_DIR}/io/hl_bina.cpp"
    "${HEDGELIB_SOURCE_DIR}/io/hl_file.cpp"
    "${HEDGELIB_SOURCE_DIR}/io/hl_hh_mirage.cpp"
    "${HEDGELIB_SOURCE_DIR}/io/hl_mem_stream.cpp"
    "${HEDGELIB_SOURCE_DIR}/io/hl_path.cpp"
    "${HEDGELIB_SOURCE_DIR}/io/hl_stream.cpp"
    "${HEDGELIB_SOURCE_DIR}/materials/hl_hh_material.cpp"
    "${HEDGELIB_SOURCE_DIR}/models/hl_hh_model.cpp"
    "${HEDGELIB_SOURCE_DIR}/shader/hl_hh_shader.cpp"
    "${HEDGELIB_SOURCE_DIR}/shader/hl_shader.cpp"
    "${HEDGELIB_SOURCE_DIR}/textures/hl_hh_texture.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_blob.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_compression.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_in_blob.h"
    "${HEDGELIB_SOURCE_DIR}/hl_in_posix.h"
    "${HEDGELIB_SOURCE_DIR}/hl_in_tool_common_text.h"
    "${HEDGELIB_SOURCE_DIR}/hl_in_win32.h"
    "${HEDGELIB_SOURCE_DIR}/hl_internal.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_math.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_scene.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_text.cpp"
    "${HEDGELIB_SOURCE_DIR}/hl_tool_helpers.cpp"
    )

# Setup library
add_library(HedgeLib ${HEDGELIB_INCLUDES} ${HEDGELIB_SOURCES})
add_library(HedgeLib::HedgeLib ALIAS HedgeLib)

target_include_directories(HedgeLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${HEDGELIB_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${HEDGELIB_INCLUDE_DIR}>
    PRIVATE
        ${HEDGELIB_DEPEND_INCLUDES})

if(HEDGELIB_DISABLE_INTRINSICS)
    # Disable intrinsics if requested
    target_compile_definitions(HedgeLib PUBLIC HL_DISABLE_INTRINSICS)
endif()

if(WIN32)
    if(HEDGELIB_WIN32_FORCE_ANSI)
        # Force usage of ANSI Win32 API if requested
        target_compile_definitions(HedgeLib PUBLIC HL_WIN32_FORCE_ANSI)
    endif()
endif()

if(UNIX)
    # Use "libhedge" instead of "libHedgeLib" on Unix-based
    # platforms to follow Unix naming conventions
    set_target_properties(HedgeLib PROPERTIES OUTPUT_NAME "hedge")
endif()

# Setup dependencies
target_link_libraries(HedgeLib ${HEDGELIB_DEPEND_LIBS} ${HEDGELIB_DEPEND_LIBS_NO_INSTALL})

# Setup install
include(GNUInstallDirs)
install(TARGETS HedgeLib ${HEDGELIB_DEPEND_LIBS}
    EXPORT HedgeLib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY ${HEDGELIB_INCLUDE_DIR}/hedgelib DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT HedgeLib
    FILE HedgeLibConfig.cmake
    NAMESPACE HedgeLib::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HedgeLib)
