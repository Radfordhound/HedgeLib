# Set directories
set(HEDGERENDER_INCLUDE_DIR "include")

# Add HedgeRender dependencies
add_subdirectory(base)
add_subdirectory(gfx)
add_subdirectory(ext)

# Setup library
add_library(HedgeRender INTERFACE)
add_library(HedgeRender::HedgeRender ALIAS HedgeRender)

# Setup dependencies
set(HEDGERENDER_INTERFACE_DEPEND_LIBS
    HedgeRender_Base
    HedgeRender_GFX
)

target_link_libraries(HedgeRender INTERFACE ${HEDGERENDER_INTERFACE_DEPEND_LIBS})

# Setup CMake package config files
install(TARGETS HedgeRender
    EXPORT HedgeRenderTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(HEDGERENDER_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/HedgeRender)

install(EXPORT HedgeRenderTargets
    FILE HedgeRenderTargets.cmake
    NAMESPACE HedgeRender::
    DESTINATION ${HEDGERENDER_INSTALL_CONFIGDIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRenderConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/HedgeRenderConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRenderConfig.cmake
    INSTALL_DESTINATION ${HEDGERENDER_INSTALL_CONFIGDIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRenderConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/HedgeRenderConfigVersion.cmake
    DESTINATION ${HEDGERENDER_INSTALL_CONFIGDIR}
)

# Export HedgeRender
export(EXPORT HedgeRenderTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/HedgeRenderTargets.cmake
    NAMESPACE HedgeRender::
)

export(PACKAGE HedgeRender)
