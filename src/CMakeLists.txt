cmake_minimum_required(VERSION 3.10)

project(ipc VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

add_subdirectory(os/common)
add_subdirectory(os/dbg)
add_subdirectory(os/osal)
add_subdirectory(os/osac)
add_subdirectory(os/worker)

file(
    GLOB
    SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/port/shm/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/port/net/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/proxy/shm/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/proxy/net/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/broker/shm/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/broker/net/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/broker/ipc_broker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp
)

if(NOT MSVC_VERSION)
    file(GLOB DBUS_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dbus/*.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/port/dbus/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/proxy/dbus/*.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/broker/dbus/*.cpp
    )
endif()

add_library(${PROJECT_NAME} SHARED ${SRC_FILES} ${DBUS_SRCS})
target_link_libraries(${PROJECT_NAME} PUBLIC common dbg osal osac worker)

if(NOT MSVC_VERSION)
    target_link_libraries(${PROJECT_NAME} PUBLIC pthread sdbus-c++)
endif()

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
           ${CMAKE_CURRENT_SOURCE_DIR}/port/
           ${CMAKE_CURRENT_SOURCE_DIR}/proxy/
           ${CMAKE_CURRENT_SOURCE_DIR}/broker/
           ${CMAKE_CURRENT_SOURCE_DIR}/
           ${CMAKE_CURRENT_SOURCE_DIR}/os/
           ${CMAKE_CURRENT_SOURCE_DIR}/os/common
           ${CMAKE_CURRENT_SOURCE_DIR}/os/dbg
           ${CMAKE_CURRENT_SOURCE_DIR}/os/osal
           ${CMAKE_CURRENT_SOURCE_DIR}/os/osac
           ${CMAKE_CURRENT_SOURCE_DIR}/os/worker
)
