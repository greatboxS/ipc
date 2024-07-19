cmake_minimum_required(VERSION 3.16)

file(GLOB INF_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../include/concurrent/*.h )

set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/mesg_p.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/eventloop_p.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/eventloop_manager_p.cpp)


set(INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set(INF_DIRS "${CMAKE_INSTALL_INCLUDEDIR}/message")

add_library(concurrent SHARED ${SRC_FILES})

target_include_directories(concurrent PUBLIC "$<BUILD_INTERFACE:${INC_DIRS}>"
                                             "$<INSTALL_INTERFACE:${INF_DIRS}>")



# Installation rules
include(GNUInstallDirs)  # Load GNU standard install directories

install(TARGETS concurrent
    EXPORT ipc-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(FILES ${INF_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/concurrent
)
