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

# Setup install for HedgeSet
install(TARGETS HedgeSet
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
