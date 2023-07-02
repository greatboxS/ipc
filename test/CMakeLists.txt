cmake_minimum_required(VERSION 3.10)

project(ipc_test VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_compile_options(-g)

file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(client client.cpp)
add_executable(server server.cpp)
add_executable(main main.cpp)

target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(client PUBLIC ipc)
target_link_libraries(server PUBLIC ipc)
target_link_libraries(main PUBLIC ipc)

add_dependencies(client ipc)
add_dependencies(server ipc)
add_dependencies(main ipc)

