# Set directories
set(HEDGERENDER_GFX_INCLUDE_DIR "../${HEDGERENDER_INCLUDE_DIR}/hedgerender")
set(HEDGERENDER_GFX_SHADER_DIR "shader")
set(HEDGERENDER_GFX_SOURCE_DIR "src")

# Set includes
set(HEDGERENDER_GFX_INCLUDES
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_adapter.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_cmd_list.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_color.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_gfx_internal.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_instance.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_pipeline.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_render_device.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_render_graph.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_renderer.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_resource.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_shader.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_surface.h"
    "${HEDGERENDER_GFX_INCLUDE_DIR}/gfx/hr_upload_batch.h"
)

# Set sources
set(HEDGERENDER_GFX_SOURCES
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_adapter.cpp"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_cmd_list.cpp"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_in_adapter.h"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_in_gfx_pch.h"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_in_instance.h"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_in_render_graph.h"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_in_resource.h"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_in_surface.h"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_instance.cpp"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_pipeline.cpp"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_render_device.cpp"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_render_graph.cpp"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_renderer.cpp"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_resource.cpp"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_shader.cpp"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_surface.cpp"
    "${HEDGERENDER_GFX_SOURCE_DIR}/hr_upload_batch.cpp"
)

# Set vertex shaders
set(HEDGERENDER_GFX_VS_SHADERS
    "default_vs"
)

# Set pixel shaders
set(HEDGERENDER_GFX_PS_SHADERS
    "default_ps"
)

# Set vertex shader sources
foreach(VS_SHADER IN LISTS HEDGERENDER_GFX_VS_SHADERS)
    list(APPEND HEDGERENDER_GFX_SOURCES
        "${HEDGERENDER_GFX_SHADER_DIR}/${VS_SHADER}.hlsl")
endforeach()

# Set pixel shader sources
foreach(PS_SHADER IN LISTS HEDGERENDER_GFX_PS_SHADERS)
    list(APPEND HEDGERENDER_GFX_SOURCES
        "${HEDGERENDER_GFX_SHADER_DIR}/${PS_SHADER}.hlsl")
endforeach()

# Set basic public include directories
set(HEDGERENDER_GFX_PUBLIC_INCLUDE_DIRS
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../${HEDGERENDER_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Setup library
add_library(HedgeRender_GFX ${HEDGERENDER_GFX_INCLUDES} ${HEDGERENDER_GFX_SOURCES})
add_library(HedgeRender::HedgeRender_GFX ALIAS HedgeRender_GFX)

set_target_properties(HedgeRender_GFX PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    ${HEDGELIB_USE_DEBUG_POSTFIX}
    FOLDER HedgeRender
)

target_precompile_headers(HedgeRender_GFX
    PRIVATE "${HEDGERENDER_GFX_SOURCE_DIR}/hr_in_gfx_pch.h"
)

# Setup DLL preprocessor definitions
if(BUILD_SHARED_LIBS)
    target_compile_definitions(HedgeRender_GFX
        PUBLIC HR_GFX_IS_DLL
        PRIVATE HR_GFX_IS_BUILDING_DLL
    )
endif()

# Setup platform-specific stuff
if(WIN32)
    if(NOT HEDGELIB_WIN32_FORCE_ANSI)
        target_compile_definitions(HedgeRender_GFX
            PRIVATE
                _UNICODE
                UNICODE
        )
    endif()
endif()

# Setup shaders
function(add_spirv_shader SHADER_STAGE SHADER_NAME)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.h
        MAIN_DEPENDENCY ${HEDGERENDER_GFX_SHADER_DIR}/${SHADER_NAME}.hlsl
        COMMAND glslc -fshader-stage=${SHADER_STAGE} -mfmt=num ${CMAKE_CURRENT_SOURCE_DIR}/${HEDGERENDER_GFX_SHADER_DIR}/${SHADER_NAME}.hlsl
            -o "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.h"
    )
endfunction()

foreach(VS_SHADER IN LISTS HEDGERENDER_GFX_VS_SHADERS)
    add_spirv_shader(vertex ${VS_SHADER})
endforeach()

foreach(PS_SHADER IN LISTS HEDGERENDER_GFX_PS_SHADERS)
    add_spirv_shader(fragment ${PS_SHADER})
endforeach()

list(APPEND HEDGERENDER_GFX_PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

# Find robin_hood and add it to HedgeRender_GFX dependencies
if(NOT TARGET robin_hood::robin_hood)
    message(STATUS "Searching for robin_hood...")
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
    find_package(robin_hood QUIET REQUIRED)
    unset(CMAKE_FIND_PACKAGE_PREFER_CONFIG)
endif()

list(APPEND HEDGERENDER_GFX_PUBLIC_DEPEND_LIBS robin_hood::robin_hood)

# Find Vulkan and add it to HedgeRender_GFX dependencies
find_package(Vulkan REQUIRED)
list(APPEND HEDGERENDER_GFX_PUBLIC_DEPEND_LIBS Vulkan::Vulkan)

# Download VulkanMemoryAllocator
message(STATUS "Downloading and/or configuring VulkanMemoryAllocator as necessary...")
FetchContent_Declare(VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG origin/master
    GIT_SHALLOW TRUE
)

# Setup VulkanMemoryAllocator and add it to HedgeRender_GFX private includes
FetchContent_Populate(VulkanMemoryAllocator)
list(APPEND HEDGERENDER_GFX_PRIVATE_INCLUDE_DIRS ${vulkanmemoryallocator_SOURCE_DIR}/include)

# Setup dependencies
list(APPEND HEDGERENDER_GFX_PUBLIC_DEPEND_LIBS
    HedgeRender_Base
)

target_include_directories(HedgeRender_GFX
    PUBLIC
        ${HEDGERENDER_GFX_PUBLIC_INCLUDE_DIRS}
    PRIVATE
        ${HEDGERENDER_GFX_PRIVATE_INCLUDE_DIRS}
    )

target_link_libraries(HedgeRender_GFX
    PUBLIC ${HEDGERENDER_GFX_PUBLIC_DEPEND_LIBS}
    PRIVATE ${HEDGERENDER_GFX_PRIVATE_DEPEND_LIBS}
)

# Install include files
install(DIRECTORY ${HEDGERENDER_GFX_INCLUDE_DIR}/gfx
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hedgerender
)

install(FILES ${HEDGERENDER_GFX_INSTALL_INCLUDE_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Setup CMake package config files
install(TARGETS HedgeRender_GFX
    EXPORT HedgeRender_GFXTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(HEDGERENDER_GFX_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/HedgeRender_GFX)

install(EXPORT HedgeRender_GFXTargets
    FILE HedgeRender_GFXTargets.cmake
    NAMESPACE HedgeRender::
    DESTINATION ${HEDGERENDER_GFX_INSTALL_CONFIGDIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_GFXConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/HedgeRender_GFXConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_GFXConfig.cmake
    INSTALL_DESTINATION ${HEDGERENDER_GFX_INSTALL_CONFIGDIR}
)

set(HEDGERENDER_GFX_INSTALL_CONFIG_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_GFXConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_GFXConfigVersion.cmake
)

if(NOT BUILD_SHARED_LIBS)
    list(APPEND HEDGERENDER_GFX_INSTALL_CONFIG_FILES
        ${PROJECT_SOURCE_DIR}/cmake/modules/Findrobin_hood.cmake
    )
endif()

install(FILES
    ${HEDGERENDER_GFX_INSTALL_CONFIG_FILES}
    DESTINATION ${HEDGERENDER_GFX_INSTALL_CONFIGDIR}
)

# Export HedgeRender_GFX
export(EXPORT HedgeRender_GFXTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_GFXTargets.cmake
    NAMESPACE HedgeRender::
)

export(PACKAGE HedgeRender_GFX)
