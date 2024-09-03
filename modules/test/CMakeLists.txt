project(test VERSION 1.0.0)

# Include the directory containing additional CMake configuration files
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_PREFIX}/cmake")

add_compile_options("-g")

add_executable(${PROJECT_NAME} main.cpp)

add_executable(concurrent_test test_concurrent.cpp)

add_dependencies(${PROJECT_NAME} concurrent)

target_link_libraries(${PROJECT_NAME} PRIVATE concurrent 
                                              debuger
                                              shared_mem
                                              message_queue
                                              mutex_lock
                                              pthread)

target_link_libraries(concurrent_test PRIVATE concurrent 
                                              debuger
                                              shared_mem
                                              message_queue
                                              mutex_lock
                                              pthread)

# find_package(ipc COMPONENTS core)
# target_link_libraries(${PROJECT_NAME} PRIVATE ipc::core)