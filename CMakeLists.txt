cmake_minimum_required (VERSION 3.0)
project(libshmmalloc)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/modules")
#include(CodeCoverage)
#APPEND_COVERAGE_COMPILER_FLAGS("-g -Wall -Wno-unused-function")
set(CMAKE_C_FLAGS "-g -Wall -Wno-unused-function")
set(CMAKE_CXX_FLAGS "-g -Wall -Wno-unused-function")

add_subdirectory(src)
add_subdirectory(example)
add_subdirectory(test)
