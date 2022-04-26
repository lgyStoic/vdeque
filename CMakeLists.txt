cmake_minimum_required(VERSION 3.10)


set(CMAKE_CXX_STANDARD 11)

project(VDEQUE)

set(SRCS deque.h deque_iterator.h)

add_library(VDEQUE INTERFACE ${SRCS})

target_include_directories(VDEQUE INTERFACE ${SRCS})

add_executable(VDEQUE_TEST main.cpp)
