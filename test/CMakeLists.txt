cmake_minimum_required (VERSION 3.0)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
find_package(GTest REQUIRED)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)
add_executable(unit_test ${SRC})
target_link_libraries(unit_test gtest shmmalloc pthread)
