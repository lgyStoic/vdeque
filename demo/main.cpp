#include <iostream>

#include <Deque.h>
#include <LockfreeQueue.h>
#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <queue>

using namespace fdt;

const int ITER_TIME = 30000;
static void deque_message_send_and_receive() {
    fdt::LockfreeQueue<int> q(100);
    std::vector<int> v;
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
            v.push_back(q.front());
            q.pop_front();
        }
    });
    th1.join();
    th2.join();


    for(int i = 0; i < ITER_TIME; i++) {
      if(v[i] != i) {
        std::cout << i << " " << v[i] << std::endl;
        std::cout << "error" << std::endl;
        throw "lock free error";
      }
    }
    std::cout << std::endl;
}

static void std_deque_message_send_and_receive() {
    std::queue<int> q;
    std::mutex m;
    std::vector<int> v;
    std::thread th1([&]{
        for(int i = 0; i < ITER_TIME; i++) {
            std::lock_guard<std::mutex> guard(m);
            q.push(i);
        }
    });
    std::thread th2([&]{
        for(int i = 0; i < ITER_TIME; i++) {
            while(q.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            std::lock_guard<std::mutex> guard(m);
            v.push(q.front());
            q.pop();
        }
    });
    th1.join();
    th2.join();


    for(int i = 0; i < ITER_TIME; i++) {
      if(v[i] != i) {
        std::cout << "error" << std::endl;
        throw "lock free error";
      }
    }
    std::cout << std::endl;
}




int main() {
  Deque<int> deque;
  deque.push_front(1);
  deque.push_back(2);
  deque.pop_front();
  deque.push_front(1);
  deque.push_back(4);
  deque.pop_back();
  deque.push_back(3);
  deque.front();
  std::cout << deque << std::endl;

  deque.clear();
  for (int i = 50; i < 100; i++) {
    deque.push_back(i);
  }
  for (int i = 49; i >= 0; i--) {
    deque.push_front(i);
  }

  std::cout << deque << ' ' << deque.size() << ',' << deque.capacity() << std::endl;

  Deque<int> deque2{1, 2, 3, 4, 5};

  auto t = deque2.begin();

  std::cout << deque2.capacity() << std::endl;
  std::cout << deque2 <<std::endl;


  deque2.insert(t, 100);
  deque2.push_front(100);

  std::cout << deque2.front() << std::endl;

  for(auto &v: deque2) {
    std::cout << v << " ";
  }
  std::cout << std::endl;

  std::cout << deque2 << std::endl;
  for(int i = 0; i < 1000; i++) {
    deque_message_send_and_receive();
  }
  

  return 0;
}
