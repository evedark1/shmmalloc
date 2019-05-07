cmake_minimum_required (VERSION 3.0)
project(libshmmalloc)

set(MAKE_FLAGS "-g -Wall -Wno-unused-function")
set(CMAKE_C_FLAGS ${MAKE_FLAGS})
set(CMAKE_CXX_FLAGS ${MAKE_FLAGS})

add_subdirectory(src)
add_subdirectory(example)
add_subdirectory(test)
