#ifndef _FDT_DEQUE_LOCK_FREE_H_
#define _FDT_DEQUE_LOCK_FREE_H_

#include "DequeIterator.h"

#include <string>
#include <ostream>
#include <sstream>
#include <initializer_list>
#include <stdexcept>
#include <memory>
#include <atomic>
#include <iostream>

namespace fdt {
template<typename T> class DequeIterator;
template<typename T,  class Allocator = std::allocator<T>>
class LockfreeQueue {
public:
  LockfreeQueue();
  LockfreeQueue(size_t capacity, const Allocator& alloca = Allocator());
  LockfreeQueue(const LockfreeQueue<T>& deque, const Allocator& alloca = Allocator());
  LockfreeQueue(std::initializer_list<T> container, const Allocator& alloca = Allocator());
  ~LockfreeQueue();
  LockfreeQueue& operator=(const LockfreeQueue& deque);
  LockfreeQueue& operator=(std::initializer_list<T> container);

  void push_back(T value);
  void pop_front();
  void clear();

  T& front();
  T& at(size_t index);
  T& operator[](size_t index);
  T front() const;
  T at(size_t index) const;
  T operator[](size_t index) const;

  DequeIterator<T> begin() const;
  DequeIterator<T> end() const;
  void reserve(size_t capacity) ;

  size_t capacity() const;
  size_t size() const;
  bool empty() const;
  std::string to_string() const;

  template <typename U>
  friend std::ostream& operator<<(std::ostream& out, const LockfreeQueue<U>& deque);

private:
  T* container_;
  Allocator alloca_;
  
  size_t capacity_;
  std::atomic<size_t> size_;
  std::atomic<size_t> front_;
  std::atomic<size_t> tail_;


  static const size_t DEFAULT_CAPACITY = 64;

  void out_of_range(const char*, size_t, const char*, const char*, size_t) const;
  void check_nonempty() const;
};

template <typename T, class Allocator> 
LockfreeQueue<T, Allocator>::LockfreeQueue() : LockfreeQueue(DEFAULT_CAPACITY) {}

template <typename T, class Allocator> 
LockfreeQueue<T, Allocator>::LockfreeQueue(size_t capacity, const Allocator& alloca)
    : alloca_(alloca), capacity_(capacity), size_(0), front_(0), tail_(0) {
  container_ = alloca_.allocate(capacity_);
}

template <typename T, class Allocator> 
LockfreeQueue<T, Allocator>::LockfreeQueue(const LockfreeQueue<T>& deque, const Allocator& alloca)
    : LockfreeQueue(deque.capacity_, alloca) {
  size_t i = 0;
  for (const T& value : deque) {
    container_[i++] = value;
  }
  size_.store(deque.size_);
  tail_.store(deque.size);
}

template <typename T, class Allocator> 
LockfreeQueue<T, Allocator>::LockfreeQueue(std::initializer_list<T> container, const Allocator &alloca)
    : LockfreeQueue(container.size() * 2, alloca) {
  size_t i = 0;
  for (const T& value : container) {
    container_[i++] = value;
  }
  size_.store(container.size());
  tail_.store(container.size());
}

template <typename T, class Allocator> 
LockfreeQueue<T, Allocator>::~LockfreeQueue() {
  alloca_.deallocate(container_, capacity_);
}

template <typename T, class Allocator> 
LockfreeQueue<T, Allocator>& LockfreeQueue<T, Allocator>::operator=(const LockfreeQueue<T, Allocator>& deque) {
  size_.store(deque.size_.load());
  tail_.store(deque.size);
  front_.store(0);
  if (capacity_ < deque.capacity_) {
    alloca_.deallocate(container_, capacity_);
    capacity_ = deque.capacity_ * 2;

    container_ = alloca_.allocate(capacity_);
  }
  size_t i = 0;
  for (const T& value : deque) {
    container_[i++] = value;
  }
  return *this;
}

template <typename T, class Allocator> 
LockfreeQueue<T, Allocator>& LockfreeQueue<T, Allocator>::operator=(std::initializer_list<T> container) {
  size_.store(container.size());
  tail_.store(container.size);
  front_.store(0);
  if (capacity_ < size_.load()) {
    alloca_.deallocate(container_, capacity_);
    capacity_ = size_.load() * 2;
    alloca_.allocate(capacity_);
  }
  size_t i = 0;
  for (const T& value : container) {
    container_[i++] = value;
  }
  return *this;
}

template <typename T, class Allocator> 
void LockfreeQueue<T, Allocator>::push_back(T value) {
  int tmpTL = tail_.fetch_add(1);
  container_[tmpTL] = value;
  tail_.store(tail_.load() % capacity_);
  size_.fetch_add(1);
}

template <typename T, class Allocator> 
void LockfreeQueue<T, Allocator>::pop_front() {
  check_nonempty();
  front_.fetch_add(1);
  front_.store(front_.load() % capacity_); 
  size_.fetch_add(-1);
}

template <typename T, class Allocator> 
void LockfreeQueue<T, Allocator>::reserve(size_t capacity) {
  if (capacity <= capacity_) {
    return;
  }
  T* new_container = alloca_.allocate(capacity);
  for (size_t i = 0; i < size_.load(); i++) {
    new_container[i] = container_[(i + front_) % capacity_];
  }
  alloca_.deallocate(container_, capacity_);
  container_ = new_container;
  capacity_ = capacity;
  front_.store(0);
}

template <typename T, class Allocator> 
void LockfreeQueue<T, Allocator>::clear() {
  size_.store(0);
  front_.store(0);
}

template <typename T, class Allocator> 
T& LockfreeQueue<T, Allocator>::front() {
  check_nonempty();
  return container_[front_.load()];
}

template <typename T, class Allocator> 
T& LockfreeQueue<T, Allocator>::at(size_t index) {
  if (index >= size_) {
    out_of_range("index", index, ">=", "this->size()", size_);
  }
  return operator[](index);
}

template <typename T, class Allocator> 
T& LockfreeQueue<T, Allocator>::operator[](size_t index) {
  return container_[(front_.load() + index) % capacity_];
}

template <typename T, class Allocator> 
T LockfreeQueue<T, Allocator>::front() const {
  return front();
}

template <typename T, class Allocator> 
T LockfreeQueue<T, Allocator>::at(size_t index) const {
  return at(index);
}

template <typename T, class Allocator> 
T LockfreeQueue<T, Allocator>::operator[](size_t index) const {
  return operator[](index);
}

template <typename T, class Allocator> 
DequeIterator<T> LockfreeQueue<T,  Allocator>::begin() const {
  return DequeIterator<T>(container_, capacity_, size_, front_, 0);
}

template <typename T, class Allocator> 
DequeIterator<T> LockfreeQueue<T, Allocator>::end() const {
  return DequeIterator<T>(container_, capacity_, size_, front_, size_);
}

template <typename T, class Allocator> 
size_t LockfreeQueue<T, Allocator>::capacity() const {
  return capacity_;
}

template <typename T, class Allocator> 
size_t LockfreeQueue<T, Allocator>::size() const {
  return size_.load();
}

template <typename T, class Allocator> 
bool LockfreeQueue<T, Allocator>::empty() const {
  return size_.load() == 0;
}

template <typename T, class Allocator>
std::string LockfreeQueue<T, Allocator>::to_string() const {
  std::ostringstream out;
  size_t cur = front_.load();
  size_t end = front_.load() + size_.load();
  out << "front = " << front_.load() << " ";
  out << "end = " << end % capacity_ << " ";
  out << "[ ";
  for (const T& value : *this) {
    out << value << " ";
  }
  out << "]";
  return out.str();
}

template <typename T> 
std::ostream& operator<<(std::ostream& out,
    const LockfreeQueue<T>& deque) {
  return out << deque.to_string();
}


template <typename T, class Allocator> 
void LockfreeQueue<T, Allocator>::out_of_range(const char* id_1, size_t value_1,
    const char* op, const char* id_2, size_t value_2) const {
  std::ostringstream out;
  out << "Deque: " << id_1 << " (which is " << value_1 << ") "
    << op << " " << id_2 << " (which is " << value_2 << ")";
  return;
  // throw std::out_of_range(out.str());
}

template <typename T, class Allocator> 
void LockfreeQueue<T, Allocator>::check_nonempty() const {
  if (size_.load() == 0) {
    throw std::out_of_range("Deque: cannot access element in empty deque");
  }
}
}
#endif
