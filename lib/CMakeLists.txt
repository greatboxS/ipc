cmake_minimum_required(VERSION 3.10)

project(sdbus-cpp)

set(BUILD_SHARED_LIBS ON)
set(BUILD_DOC OFF)
set(BUILD_LIBSYSTEMD OFF)
set(BUILD_CODE_GEN ON)
set(BUILD_EXAMPLES ON)

message(STATUS "Fetch sdbus-cpp package version 1.2.0")
include(FetchContent)
FetchContent_Declare(
    sdbus-cpp
    GIT_REPOSITORY https://github.com/Kistler-Group/sdbus-cpp.git
    GIT_TAG v1.2.0
)

FetchContent_MakeAvailable(sdbus-cpp)