cmake_minimum_required(VERSION 3.10)

find_package(benchmark REQUIRED)
set(BENCH_SRCS deque_bench.cpp)

add_executable(VDEQUE_BENCH ${BENCH_SRCS})

target_link_libraries(VDEQUE_BENCH PRIVATE benchmark::benchmark
                    VDEQUE)