#include <benchmark/benchmark.h>
#include <deque.h>
#include <deque>

static void BM_queue_insert_front(benchmark::State& state) {
  fdt::Deque<int> q(100);
  for (auto _ : state) {
      for (int i = 0; i < 100; i++) {
        q.push_back(i);
      }
      state.PauseTiming();
      q.clear();
      state.ResumeTiming();
  }
}

static void BM_std_queue_insert_front(benchmark::State& state) {
  std::deque<int> q(100);
  for (auto _ : state) {
      for (int i = 0; i < 100; i++) {
        q.push_back(i);
      }
      state.PauseTiming();
      q.clear();
      state.ResumeTiming();
  }
}




BENCHMARK(BM_queue_insert_front);
BENCHMARK(BM_std_queue_insert_front);
BENCHMARK_MAIN();