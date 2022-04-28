#include <benchmark/benchmark.h>
#include <Deque.h>
#include <deque>
#include <mutex>
#include <thread>
#include <chrono>
#include <queue>

const int ITER_TIME = 3000;


static void BM_queue_push_front(benchmark::State& state) {
  fdt::Deque<int> q(ITER_TIME);
  for (auto _ : state) {
      for (int i = 0; i < ITER_TIME; i++) {
        q.push_front(i);
      }
      state.PauseTiming();
      q.clear();
      state.ResumeTiming();
  }
}

static void BM_std_queue_push_front(benchmark::State& state) {
  std::deque<int> q;
  for (auto _ : state) {
      for (int i = 0; i < ITER_TIME; i++) {
        q.push_front(i);
      }
      state.PauseTiming();
      q.clear();
      state.ResumeTiming();
  }
}

static void BM_lockfree_queue_push_front(benchmark::State& state) {
  fdt::LockfreeQueue<int> q;
  for (auto _ : state) {
      for (int i = 0; i < ITER_TIME; i++) {
        q.push_back(i);
      }
      state.PauseTiming();
      q.clear();
      state.ResumeTiming();
  }
}

static void BM_queue_push_back(benchmark::State& state) {
  fdt::Deque<int> q(ITER_TIME);
  for (auto _ : state) {
      for (int i = 0; i < ITER_TIME; i++) {
        q.push_back(i);
      }
      state.PauseTiming();
      q.clear();
      state.ResumeTiming();
  }
}

static void BM_std_queue_push_back(benchmark::State& state) {
  std::deque<int> q;
  for (auto _ : state) {
      for (int i = 0; i < ITER_TIME; i++) {
        q.push_back(i);
      }
      state.PauseTiming();
      q.clear();
      state.ResumeTiming();
  }
}

static void BM_lockfree_queue_push_back(benchmark::State& state) {
  fdt::LockfreeQueue<int>  q(ITER_TIME);
  for (auto _ : state) {
      for (int i = 0; i < ITER_TIME; i++) {
        q.push_back(i);
      }
      state.PauseTiming();
      q.clear();
      state.ResumeTiming();
  }
}

static void BM_queue_pop_front(benchmark::State& state) {
    fdt::Deque<int> q(ITER_TIME);
    for(auto _ : state) {
        state.PauseTiming();
        for(int i = 0; i < ITER_TIME; i++) {
            q.push_back(i);
        }
        state.ResumeTiming();
        for(int i = 0; i < ITER_TIME; i++) {
            q.pop_front();
        }
    }
}

static void BM_std_queue_pop_front(benchmark::State& state) {
    std::deque<int> q;
    for(auto _ : state) {
        state.PauseTiming();
        for(int i = 0; i < ITER_TIME; i++) {
            q.push_back(i);
        }
        state.ResumeTiming();
        for(int i = 0; i < ITER_TIME; i++) {
            q.pop_front();
        }
    }
}


static void BM_queue_push_pop_front(benchmark::State& state) {
    fdt::Deque<int> q(ITER_TIME);
    for(auto _ : state) {
        for(int i = 0; i < ITER_TIME; i++) {
            q.push_back(i);
            benchmark::DoNotOptimize(q.front());
            q.pop_front();
            q.push_front(i);
            benchmark::DoNotOptimize(q.back());
            q.pop_back();
        }
    }
}

static void BM_std_queue_push_pop_front(benchmark::State& state) {
    std::deque<int> q;
    for(auto _ : state) {
        for(int i = 0; i < ITER_TIME; i++) {
            q.push_back(i);
            benchmark::DoNotOptimize(q.front());
            q.pop_front();
            q.push_front(i);
            benchmark::DoNotOptimize(q.back());
            q.pop_back();
        }
    }
}

static void BM_queue_message_send_and_receive(benchmark::State& state) {
    fdt::LockfreeQueue<int> q(100);
    for(auto _ : state) {
        std::thread th1([&]{
            for(int i = 0; i < ITER_TIME; i++) {
                while(q.full()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                q.push_back(i);
            }
        });
        std::thread th2([&]{
            for(int i = 0; i < ITER_TIME; i++) {
                while(q.empty()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                q.pop_front();
            }
        });
        state.PauseTiming();
        th1.join();
        th2.join();
        state.ResumeTiming();
    }
}

static void BM_std_deque_message_send_and_receive(benchmark::State& state) {
    std::queue<int> q;
    std::mutex m;
    for(auto _ : state) {
        std::thread th1([&]{
            for(int i = 0; i < ITER_TIME; i++) {
                std::lock_guard<std::mutex> guard(m);
                q.push(i);
            }
        });
        std::thread th2([&]{
            for(int i = 0; i < ITER_TIME; i++) {
                std::lock_guard<std::mutex> guard(m);
                while(q.empty()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                q.pop();
            }
        });
        state.PauseTiming();
        th1.join();
        th2.join();
        state.ResumeTiming();
    }
}

// BENCHMARK(BM_queue_push_front);
// BENCHMARK(BM_std_queue_push_front);
// BENCHMARK(BM_lockfree_queue_push_front);
// BENCHMARK(BM_queue_push_back);
// BENCHMARK(BM_std_queue_push_back);
// BENCHMARK(BM_lockfree_queue_push_back);
// BENCHMARK(BM_queue_pop_front);
// BENCHMARK(BM_std_queue_pop_front);
// BENCHMARK(BM_queue_push_pop_front);
// BENCHMARK(BM_std_queue_push_pop_front);
BENCHMARK(BM_queue_message_send_and_receive);
BENCHMARK(BM_std_deque_message_send_and_receive);


BENCHMARK_MAIN();