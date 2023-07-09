cmake_minimum_required(VERSION 3.10)

project(dbg VERSION 1.0)

file(GLOB SRC_FILES Debug.cpp Exception.cpp Backtrace.cpp)

add_library(${PROJECT_NAME} OBJECT ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC common)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../)

# Set all header file to be install later
file(GLOB HEADER_FILES ${PROJECT_SOURCE_DIR}/*.h ${PROJECT_SOURCE_DIR}/*.hpp)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")
