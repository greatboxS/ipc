cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

project(ipc VERSION 1.0.0 LANGUAGES CXX)

add_subdirectory(debuger)
add_subdirectory(exception)
add_subdirectory(core)
add_subdirectory(concurrent)
add_subdirectory(ipc)
add_subdirectory(test)

# Custom command to build and install the library first, then run tests
add_custom_target(build_and_test
    COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target install
    COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --output-on-failure
    DEPENDS message  # Ensure message library is built before running tests
    COMMENT "Building and installing library, then running tests"
)

# Alias for `make test` to run the tests after installation
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --output-on-failure)

# Export targets for the IPC module
export(EXPORT ipc-targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/ipc-targets.cmake"
    NAMESPACE ipc::
)

# Install IPC module configuration files
install(EXPORT ipc-targets
    FILE ipc-targets.cmake
    NAMESPACE ipc::
    DESTINATION lib/cmake/ipc
)

# Configure and install ipc-config.cmake
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ipc-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/ipc-config.cmake"
    INSTALL_DESTINATION lib/cmake/ipc
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/ipc-config.cmake"
              cmake/ipc-config-version.cmake
        DESTINATION lib/cmake/ipc
)
