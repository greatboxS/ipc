cmake_minimum_required(VERSION 3.10)

project(gbs-ipc VERSION 1.0.0)

add_compile_options("-fPIC")

add_subdirectory(lib)
add_subdirectory(modules)
