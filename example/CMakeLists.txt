cmake_minimum_required (VERSION 3.0)

include_directories(${PROJECT_SOURCE_DIR}/src)
add_executable(shm_example main.c)
target_link_libraries(shm_example shmmalloc pthread)
