cmake_minimum_required(VERSION 3.10)

project(gbs-ipc VERSION 1.0.0)

if(NOT MSVC_VERSION)
    add_subdirectory(lib)
endif()
add_subdirectory(src)
add_subdirectory(test)
