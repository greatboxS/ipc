cmake_minimum_required(VERSION 3.10)

# Project name and version
project(test VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable external project capabilities
include(FetchContent)

# Fetch GoogleTest and install it as part of the build
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

# Only needed if testing - this is optional, so include if tests are enabled
FetchContent_MakeAvailable(googletest)

add_compile_options("-g")

if(EXISTS ${CMAKE_INSTALL_PREFIX}/lib)
    add_subdirectory(lib_test)
endif()

# task testing
add_executable(task_test task_test.cpp)
add_executable(concurrent_test concurrent_test.cpp)

add_executable(test_concurrent test_concurrent.cpp)

target_link_libraries(task_test PRIVATE gtest_main concurrent 
                                              debuger
                                              shared_mem
                                              message_queue
                                              mutex_lock
                                              pthread)

target_link_libraries(concurrent_test PRIVATE gtest_main concurrent 
                                              debuger
                                              shared_mem
                                              message_queue
                                              mutex_lock
                                              pthread)

target_link_libraries(test_concurrent PRIVATE concurrent 
                                              debuger
                                              shared_mem
                                              message_queue
                                              mutex_lock
                                              pthread)

# Add Google Test and your tests to CMake
enable_testing()
add_test(NAME task_test COMMAND task_test)

add_test(NAME concurrent_test COMMAND concurrent_test)

# Add a target to build the tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS task_test concurrent_test)


# Include the directory containing additional CMake configuration files
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_PREFIX}/cmake")
