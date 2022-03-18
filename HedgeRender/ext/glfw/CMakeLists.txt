# Set directories
set(HEDGERENDER_EXT_GLFW_INCLUDE_DIR "../../${HEDGERENDER_INCLUDE_DIR}/hedgerender")

# Set includes
set(HEDGERENDER_EXT_GLFW_INCLUDES
    "${HEDGERENDER_EXT_GLFW_INCLUDE_DIR}/ext/hr_ext_glfw.h"
)

# Set basic public include directories
set(HEDGERENDER_EXT_GLFW_INTERFACE_INCLUDE_DIRS
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../${HEDGERENDER_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Setup library
add_library(HedgeRender_Ext_GLFW INTERFACE ${HEDGERENDER_EXT_GLFW_INCLUDES})
add_library(HedgeRender::HedgeRender_Ext_GLFW ALIAS HedgeRender_Ext_GLFW)

set_target_properties(HedgeRender_Ext_GLFW PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    FOLDER HedgeRender
)

# Setup platform-specific stuff
if(WIN32)
    if(NOT HEDGELIB_WIN32_FORCE_ANSI)
        target_compile_definitions(HedgeRender_Ext_GLFW
            INTERFACE
                _UNICODE    
                UNICODE
        )
    endif()
endif()

# Setup dependencies
list(APPEND HEDGERENDER_EXT_GLFW_INTERFACE_DEPEND_LIBS
    HedgeRender_GFX
)

target_include_directories(HedgeRender_Ext_GLFW
    INTERFACE
        ${HEDGERENDER_EXT_GLFW_INTERFACE_INCLUDE_DIRS}
    )

target_link_libraries(HedgeRender_Ext_GLFW
    INTERFACE ${HEDGERENDER_EXT_GLFW_INTERFACE_DEPEND_LIBS}
)

# Install include files
install(DIRECTORY ${HEDGERENDER_EXT_GLFW_INCLUDE_DIR}/ext
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hedgerender
)

install(FILES ${HEDGERENDER_EXT_GLFW_INSTALL_INCLUDE_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Setup CMake package config files
install(TARGETS HedgeRender_Ext_GLFW
    EXPORT HedgeRender_Ext_GLFWTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(HEDGERENDER_EXT_GLFW_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/HedgeRender_Ext_GLFW)

install(EXPORT HedgeRender_Ext_GLFWTargets
    FILE HedgeRender_Ext_GLFWTargets.cmake
    NAMESPACE HedgeRender::
    DESTINATION ${HEDGERENDER_EXT_GLFW_INSTALL_CONFIGDIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_Ext_GLFWConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/HedgeRender_Ext_GLFWConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_Ext_GLFWConfig.cmake
    INSTALL_DESTINATION ${HEDGERENDER_EXT_GLFW_INSTALL_CONFIGDIR}
)

set(HEDGERENDER_EXT_GLFW_INSTALL_CONFIG_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_Ext_GLFWConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_Ext_GLFWConfigVersion.cmake
)

install(FILES
    ${HEDGERENDER_EXT_GLFW_INSTALL_CONFIG_FILES}
    DESTINATION ${HEDGERENDER_EXT_GLFW_INSTALL_CONFIGDIR}
)

# Export HedgeRender_Ext_GLFW
export(EXPORT HedgeRender_Ext_GLFWTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/HedgeRender_Ext_GLFWTargets.cmake
    NAMESPACE HedgeRender::
)

export(PACKAGE HedgeRender_Ext_GLFW)
