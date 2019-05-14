cmake_minimum_required (VERSION 3.0)

include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)
add_library(shmmalloc ${SRC})
