cmake_minimum_required(VERSION 3.10)

project(osac VERSION 1.0)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.c)
file(GLOB HEADER_FILES ${PROJECT_SOURCE_DIR}/*.h ${PROJECT_SOURCE_DIR}/*.hpp)

add_library(${PROJECT_NAME} OBJECT ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC osal)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../)

# Set all header file to be install later
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")
