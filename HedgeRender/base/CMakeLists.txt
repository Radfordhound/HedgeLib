# Set directories
set(HEDGERENDER_BASE_INCLUDE_DIR "../${HEDGERENDER_INCLUDE_DIR}/hedgerender")
set(HEDGERENDER_BASE_SOURCE_DIR "src")

# Set includes
set(HEDGERENDER_BASE_INCLUDES
    "${HEDGERENDER_BASE_INCLUDE_DIR}/base/hr_array.h"
    "${HEDGERENDER_BASE_INCLUDE_DIR}/base/hr_base_internal.h"
    "${HEDGERENDER_BASE_INCLUDE_DIR}/base/hr_log.h"
)

# Set sources
set(HEDGERENDER_BASE_SOURCES
    "${HEDGERENDER_BASE_SOURCE_DIR}/hr_in_base_pch.h"
    "${HEDGERENDER_BASE_SOURCE_DIR}/hr_log.cpp"
)

# Setup library
add_library(HedgeRender_Base ${HEDGERENDER_BASE_INCLUDES} ${HEDGERENDER_BASE_SOURCES})
add_library(HedgeRender::HedgeRender_Base ALIAS HedgeRender_Base)

set_target_properties(HedgeRender_Base PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    ${HEDGELIB_USE_DEBUG_POSTFIX}
    FOLDER HedgeRender
)

target_precompile_headers(HedgeRender_Base
    PRIVATE "${HEDGERENDER_BASE_SOURCE_DIR}/hr_in_base_pch.h"
 )

# Setup DLL preprocessor definitions
if(BUILD_SHARED_LIBS)
    target_compile_definitions(HedgeRender_Base
        PUBLIC HR_BASE_IS_DLL
        PRIVATE HR_BASE_IS_BUILDING_DLL
    )
endif()

# Setup platform-specific stuff
if(WIN32)
    if(NOT HEDGELIB_WIN32_FORCE_ANSI)
        target_compile_definitions(HedgeRender_Base
            PRIVATE
                _UNICODE    
                UNICODE
        )
    endif()
endif()

# Setup dependencies
set(HEDGERENDER_BASE_PUBLIC_DEPEND_LIBS
    HedgeLib
)

target_include_directories(HedgeRender_Base
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../${HEDGERENDER_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(HedgeRender_Base
    PUBLIC ${HEDGERENDER_BASE_PUBLIC_DEPEND_LIBS}
)

# Install include files
install(DIRECTORY ${HEDGERENDER_BASE_INCLUDE_DIR}/base
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hedgerender
)

# Setup CMake package config files
install(TARGETS HedgeRender_Base
    EXPORT HedgeRender_BaseTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(HEDGERENDER_BASE_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/HedgeRender_Base)

install(EXPORT HedgeRender_BaseTargets
    FILE HedgeRender_BaseTargets.cmake
    NAMESPACE HedgeRender::
    DESTINATION ${HEDGERENDER_BASE_INSTALL_CONFIGDIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_BaseConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/HedgeRender_BaseConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_BaseConfig.cmake
    INSTALL_DESTINATION ${HEDGERENDER_BASE_INSTALL_CONFIGDIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_BaseConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_BaseConfigVersion.cmake
    DESTINATION ${HEDGERENDER_BASE_INSTALL_CONFIGDIR}
)

# Export HedgeRender_Base
export(EXPORT HedgeRender_BaseTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_BaseTargets.cmake
    NAMESPACE HedgeRender::
)

export(PACKAGE HedgeRender_Base)
