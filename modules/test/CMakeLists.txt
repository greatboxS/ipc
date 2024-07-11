project(IPCTest VERSION 1.0.0)

# Include the directory containing additional CMake configuration files
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_PREFIX}/cmake")


add_executable(${PROJECT_NAME} main.cpp)

add_dependencies(${PROJECT_NAME} core)

find_package(ipc COMPONENTS core)
target_link_libraries(${PROJECT_NAME} PRIVATE ipc::core)