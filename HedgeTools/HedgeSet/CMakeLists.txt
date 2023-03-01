# Set directories
set(HEDGESET_SOURCE_DIR "src")

# Set sources
set(HEDGESET_SOURCES
    "${HEDGESET_SOURCE_DIR}/main.cpp"
)

# Setup executable
add_executable(HedgeSet ${HEDGESET_SOURCES})
target_link_libraries(HedgeSet HedgeLib)

set_target_properties(HedgeSet PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    FOLDER HedgeTools
)

# Copy templates on post-build on Windows
if(WIN32)
message(STATUS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/templates)
    add_custom_command(TARGET HedgeSet
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/Templates/
            ${CMAKE_BINARY_DIR}/$<CONFIG>/templates
    )
endif()

# Setup install for HedgeSet
install(TARGETS HedgeSet
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
