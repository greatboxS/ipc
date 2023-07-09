cmake_minimum_required(VERSION 3.10)

project(osal VERSION 1.0)

# Get source files based on selection platform
if(${PLATFORM} MATCHES "linux")
    file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/linux/*.cpp ${PROJECT_SOURCE_DIR}/linux/*.c)
elseif(${PLATFORM} MATCHES "windows")
    file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/windows/*.cpp ${PROJECT_SOURCE_DIR}/windows/*.c)
else()

endif()

# Get all header files
file(GLOB HEADER_FILES ${PROJECT_SOURCE_DIR}/*.h ${PROJECT_SOURCE_DIR}/*.hpp)
file(GLOB COMMON_SRC_FILES ${PROJECT_SOURCE_DIR}/*.c ${PROJECT_SOURCE_DIR}/*.cpp)

list(APPEND SRC_FILES ${COMMON_SRC_FILES})

# Add osal build object files
add_library(${PROJECT_NAME} OBJECT ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC dbg)

# Add local link libraries
# target_link_libraries(${PROJECT_NAME} PUBLIC common dbg)

# Add link library
if(${PLATFORM} MATCHES "linux")
    target_link_libraries(${PROJECT_NAME} PUBLIC -pthread -lrt)
elseif(${PLATFORM} MATCHES "windows")
    target_link_libraries(${PROJECT_NAME} PUBLIC wsock32 ws2_32)
else()

endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../)

# Set all header file to be install later
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")
