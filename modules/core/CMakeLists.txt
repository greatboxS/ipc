cmake_minimum_required(VERSION 3.20)

set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/mesg_p.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/eventloop_p.cpp)


set(INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../interface")
set(INF_DIRS "${CMAKE_INSTALL_INCLUDEDIR}/message")

add_library(core STATIC ${SRC_FILES})

target_include_directories(core PUBLIC $<BUILD_INTERFACE:${INC_DIRS}>
                                          $<INSTALL_INTERFACE:${INF_DIRS}>)

# Installation rules
include(GNUInstallDirs)  # Load GNU standard install directories

install(TARGETS core
    EXPORT ipc-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../interface/mesg.h
              ${CMAKE_CURRENT_SOURCE_DIR}/../interface/task.h
              ${CMAKE_CURRENT_SOURCE_DIR}/../interface/task_helpers.h
              ${CMAKE_CURRENT_SOURCE_DIR}/../interface/worker.h
              ${CMAKE_CURRENT_SOURCE_DIR}/../interface/worker_manager.h
              ${CMAKE_CURRENT_SOURCE_DIR}/../interface/eventloop.h
              ${CMAKE_CURRENT_SOURCE_DIR}/../interface/eventloop_manager.h
              ${CMAKE_CURRENT_SOURCE_DIR}/../interface/ipc_def.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/core
)
