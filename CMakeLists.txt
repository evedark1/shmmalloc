cmake_minimum_required (VERSION 3.0)
project(libshmmalloc)

set(CMAKE_C_FLAGS "-g -Wall")

add_subdirectory(src)
add_subdirectory(example)
add_subdirectory(test)
