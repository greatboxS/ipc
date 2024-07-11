set(PACKAGE_VERSION 1.0.0)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/ipc-config-version.cmake"
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# Install the version file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ipc-config-version.cmake"
  DESTINATION lib/cmake/ipc
)