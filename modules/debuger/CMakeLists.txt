cmake_minimum_required(VERSION 3.16)

project(debuger VERSION 1.0.0)

file(GLOB INF_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../include/debuger/*.h )

set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/debuger.cpp)


set(INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set(INF_DIRS "${CMAKE_INSTALL_INCLUDEDIR}/debuger")

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

if (${PLATFORM} MATCHES "linux")
    target_link_libraries(${PROJECT_NAME} PRIVATE dl)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${INC_DIRS}>"
                                                  "$<INSTALL_INTERFACE:${INF_DIRS}>")

# Installation rules
include(GNUInstallDirs)  # Load GNU standard install directories

set(VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS ${PROJECT_NAME}
    EXPORT ipc-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(FILES ${INF_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/debuger)
