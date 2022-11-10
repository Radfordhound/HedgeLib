# Set directories
set(HEDGENEEDLE_SOURCE_DIR "src")

# Set sources
set(HEDGENEEDLE_SOURCES
    "${HEDGENEEDLE_SOURCE_DIR}/main.cpp"
)

# Setup executable
add_executable(HedgeNeedle ${HEDGENEEDLE_SOURCES})
target_link_libraries(HedgeNeedle HedgeLib)

set_target_properties(HedgeNeedle PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    FOLDER HedgeTools
)

# Setup install for HedgeNeedle
install(TARGETS HedgeNeedle
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
