cmake_minimum_required(VERSION 3.10)
project(lib_test VERSION 1.0.0)

# Include the directory containing additional CMake configuration files
set(CMAKE_PREFIX_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_LIBDIR}/cmake")

find_package(ipc REQUIRED COMPONENTS concurrent 
                            debuger 
                            shared_mem
                            message_queue
                            mutex_lock)

add_compile_options("-g")

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ipc::concurrent 
                                              ipc::debuger
                                              ipc::shared_mem
                                              ipc::message_queue
                                              ipc::mutex_lock
                                              pthread)