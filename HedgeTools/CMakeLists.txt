add_subdirectory(HedgeArcPack)
add_subdirectory(HedgeNeedle)
add_subdirectory(HedgeSet)

# Install templates
if(WIN32) # TODO: Non-windows HedgeEdit support
    #add_subdirectory(HedgeEdit)
    set(HEDGETOOLS_TEMPLATES_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}/templates")
else()
    set(HEDGETOOLS_TEMPLATES_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/hedgetools/templates")
endif()

install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/Templates/
    DESTINATION ${HEDGETOOLS_TEMPLATES_INSTALL_DIR}
)
