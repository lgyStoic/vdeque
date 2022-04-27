cmake_minimum_required(VERSION 3.10)


set(CMAKE_CXX_STANDARD 11)

project(VDEQUE)

set(SRCS src/deque.h 
    src/deque_lockfree.h
    src/deque_iterator.h)

add_library(VDEQUE INTERFACE ${SRCS})

target_include_directories(VDEQUE INTERFACE src)

add_executable(VDEQUE_DEMO demo/main.cpp)

target_link_libraries(VDEQUE_DEMO VDEQUE)

set(BUILD_DEQUEBENCH ON)

if (BUILD_DEQUEBENCH)
    add_subdirectory(benchmark)
endif()