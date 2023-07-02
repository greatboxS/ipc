cmake_minimum_required(VERSION 3.10)

project(ipc VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

file(
    GLOB
    SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/port/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/proxy/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/broker/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC sdbus-c++)

target_include_directories(
    ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/
)
