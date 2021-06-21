# Require at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

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
set_target_properties(HedgeArcPack PROPERTIES FOLDER HedgeTools)

include(GNUInstallDirs)
install(TARGETS HedgeArcPack
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
