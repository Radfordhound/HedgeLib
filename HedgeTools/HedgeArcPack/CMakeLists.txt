# Set directories
set(HEDGEARCPACK_SOURCE_DIR "src")

# Set sources
set(HEDGEARCPACK_SOURCES
    "${HEDGEARCPACK_SOURCE_DIR}/main.cpp"
    "${HEDGEARCPACK_SOURCE_DIR}/text.h"
)

# Setup executable
add_executable(HedgeArcPack ${HEDGEARCPACK_SOURCES})
target_link_libraries(HedgeArcPack HedgeLib)

set_target_properties(HedgeArcPack PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    FOLDER HedgeTools
)

# Setup install for HedgeArcPack
install(TARGETS HedgeArcPack
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
