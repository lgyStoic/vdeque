#ifndef _FDT_DEQUE_LOCK_FREE_H_
#define _FDT_DEQUE_LOCK_FREE_H_

#include "deque_iterator.h"

#include <string>
#include <ostream>
#include <sstream>
#include <initializer_list>
#include <stdexcept>
#include <memory>
#include <atomic>

namespace fdt {
template<typename T> class DequeIterator;
template<typename T,  class Allocator = std::allocator<T>>
class LockFreeDeque {
public:
  LockFreeDeque();
  LockFreeDeque(size_t capacity, const Allocator& alloca = Allocator());
  LockFreeDeque(const LockFreeDeque<T>& deque, const Allocator& alloca = Allocator());
  LockFreeDeque(std::initializer_list<T> container, const Allocator& alloca = Allocator());
  ~LockFreeDeque();
  LockFreeDeque& operator=(const LockFreeDeque& deque);
  LockFreeDeque& operator=(std::initializer_list<T> container);

  void push_front(T value);
  void push_back(T value);
  void pop_front();
  void pop_back();
  void erase(const DequeIterator<T>& begin, const DequeIterator<T>& end);
  void erase(const DequeIterator<T>& it);
  void insert(const DequeIterator<T>& it, T value);
  void reserve(size_t);
  void resize(size_t, T = T());
  void clear();

  T& front();
  T& back();
  T& at(size_t index);
  T& operator[](size_t index);
  T front() const;
  T back() const;
  T at(size_t index) const;
  T operator[](size_t index) const;

  DequeIterator<T> begin() const;
  DequeIterator<T> end() const;

  size_t capacity() const;
  size_t size() const;
  bool empty() const;
  std::string to_string() const;

  template <typename U>
  friend std::ostream& operator<<(std::ostream& out, const LockFreeDeque<U>& deque);

private:
  T* container_;
  Allocator alloca_;
  
  size_t capacity_;
  std::atomic<size_t> size_;
  std::atomic<size_t> front_;
  std::atomic<size_t> end_;

  static const size_t DEFAULT_CAPACITY = 64;

  void reallocate();
  void shift_left(size_t, size_t, size_t);
  void shift_right(size_t, size_t, size_t);
  void out_of_range(const char*, size_t, const char*, const char*, size_t) const;
  void check_nonempty() const;
};

template <typename T, class Allocator> 
LockFreeDeque<T, Allocator>::LockFreeDeque() : LockFreeDeque(DEFAULT_CAPACITY) {}

template <typename T, class Allocator> 
LockFreeDeque<T, Allocator>::LockFreeDeque(size_t capacity, const Allocator& alloca)
    : alloca_(alloca), capacity_(capacity), size_(0), front_(0) {
  container_ = alloca_.allocate(capacity_);
}

template <typename T, class Allocator> 
LockFreeDeque<T, Allocator>::LockFreeDeque(const LockFreeDeque<T>& deque, const Allocator& alloca)
    : LockFreeDeque(deque.capacity_, alloca) {
  size_t i = 0;
  for (const T& value : deque) {
    container_[i++] = value;
  }
  size_.store(deque.size_);
}

template <typename T, class Allocator> 
LockFreeDeque<T, Allocator>::LockFreeDeque(std::initializer_list<T> container, const Allocator &alloca)
    : LockFreeDeque(container.size() * 2, alloca) {
  size_t i = 0;
  for (const T& value : container) {
    container_[i++] = value;
  }
  size_.store(container.size());
}

template <typename T, class Allocator> 
LockFreeDeque<T, Allocator>::~LockFreeDeque() {
  alloca_.deallocate(container_, capacity_);
}

template <typename T, class Allocator> 
LockFreeDeque<T, Allocator>& LockFreeDeque<T, Allocator>::operator=(const LockFreeDeque<T, Allocator>& deque) {
  size_.store(deque.size_.load());
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
LockFreeDeque<T, Allocator>& LockFreeDeque<T, Allocator>::operator=(std::initializer_list<T> container) {
  size_.store(container.size());
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
void LockFreeDeque<T, Allocator>::push_front(T value) {
  front_.store((front_.load() + + capacity_ - 1) % capacity_);
  container_[front_.load()] = value;
  size_.store(size_.load() + 1);
  reallocate();
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::push_back(T value) {
  container_[(front_.load() + size_.load()) % capacity_] = value;
  size_.store(size_.load() + 1);
  reallocate();
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::pop_front() {
  check_nonempty();
  int tmpSz = size_.load();
  int tmpFt = front_.load();
  size_.store(tmpSz - 1);
  front_.store(( tmpFt+ 1) % capacity_);
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::pop_back() {
  check_nonempty();
  size_.store(size_.load() - 1);
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::erase(const DequeIterator<T>& begin, const DequeIterator<T>& end) {
  if (begin.index_ >= size_) {
    out_of_range("begin.index_", begin.index_, ">=", "this->size()", size_);
  }
  if (end.index_ > size_) {
    out_of_range("end.index_", end.index_, ">", "this->size()", size_);
  }
  if (begin.index_ > end.index_) {
    out_of_range("begin.index_", begin.index_, ">", "end.index_", end.index_);
  }
  size_t offset = end.index_ - begin.index_;
  if (begin.index_ + end.index_ < size_.load()) {
    shift_right(0, begin.index_, offset);
    front_.store((front_.load() + offset) % capacity_);
  }
  else {
    shift_left(end.index_, size_.load(), offset);
  }
  size_.store(size_.load() - offset);
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::erase(const DequeIterator<T>& it) {
  return erase(it, it + 1);
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::insert(const DequeIterator<T>& it, T value) {
  if (it.index_ > size_) {
    out_of_range("it.index_", it.index_, ">", "this->size()", size_);
  }
  if (it.index_ < size_.load() / 2) {
    shift_left(0, it.index_, 1);
    front_.store((front_.load() + capacity_ - 1) % capacity_);
  }
  else {
    shift_right(it.index_, size_.load(), 1);
  }
  container_[(it.index_ + front_.load()) % capacity_] = value;
  size_.store(size_.load() + 1);
  reallocate();
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::reserve(size_t capacity) {
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
void LockFreeDeque<T, Allocator>::resize(size_t size, T value) {
  if (size > capacity_) {
    reserve(size);
  }
  for (size_t i = front_.load() + size_.load(); i < front_.load() + size; i++) {
    container_[i % capacity_] = value;
  }
  size_.store(size);
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::clear() {
  size_.store(0);
  front_.store(0);
}

template <typename T, class Allocator> 
T& LockFreeDeque<T, Allocator>::front() {
  check_nonempty();
  return container_[front_.load()];
}

template <typename T, class Allocator> 
T& LockFreeDeque<T, Allocator>::back() {
  check_nonempty();
  return container_[(front_.load() + size_.load() - 1) % capacity_];
}

template <typename T, class Allocator> 
T& LockFreeDeque<T, Allocator>::at(size_t index) {
  if (index >= size_) {
    out_of_range("index", index, ">=", "this->size()", size_);
  }
  return operator[](index);
}

template <typename T, class Allocator> 
T& LockFreeDeque<T, Allocator>::operator[](size_t index) {
  return container_[(front_.load() + index) % capacity_];
}

template <typename T, class Allocator> 
T LockFreeDeque<T, Allocator>::front() const {
  return front();
}

template <typename T, class Allocator> 
T LockFreeDeque<T, Allocator>::back() const {
  return back();
}

template <typename T, class Allocator> 
T LockFreeDeque<T, Allocator>::at(size_t index) const {
  return at(index);
}

template <typename T, class Allocator> 
T LockFreeDeque<T, Allocator>::operator[](size_t index) const {
  return operator[](index);
}

template <typename T, class Allocator> 
DequeIterator<T> LockFreeDeque<T,  Allocator>::begin() const {
  return DequeIterator<T>(container_, capacity_, size_, front_, 0);
}

template <typename T, class Allocator> 
DequeIterator<T> LockFreeDeque<T, Allocator>::end() const {
  return DequeIterator<T>(container_, capacity_, size_, front_, size_);
}

template <typename T, class Allocator> 
size_t LockFreeDeque<T, Allocator>::capacity() const {
  return capacity_;
}

template <typename T, class Allocator> 
size_t LockFreeDeque<T, Allocator>::size() const {
  return size_.load();
}

template <typename T, class Allocator> 
bool LockFreeDeque<T, Allocator>::empty() const {
  return size_.load() == 0;
}

template <typename T, class Allocator>
std::string LockFreeDeque<T, Allocator>::to_string() const {
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
    const LockFreeDeque<T>& deque) {
  return out << deque.to_string();
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::reallocate() {
  if (size_.load() < capacity_) {
    return;
  }
  reserve(capacity_ * 2);
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::shift_left(size_t begin, size_t end, size_t offset) {
  for (size_t i = begin + front_.load(); i < end + front_.load(); i++) {
    container_[(i - offset) % capacity_] = container_[i % capacity_];
  }
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::shift_right(size_t begin, size_t end, size_t offset) {
  for (size_t i = end + front_.load() - 1; i >= begin + front_.load(); i--) {
    container_[(i + offset) % capacity_] = container_[i % capacity_];
  }
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::out_of_range(const char* id_1, size_t value_1,
    const char* op, const char* id_2, size_t value_2) const {
  std::ostringstream out;
  out << "Deque: " << id_1 << " (which is " << value_1 << ") "
    << op << " " << id_2 << " (which is " << value_2 << ")";
  return;
  // throw std::out_of_range(out.str());
}

template <typename T, class Allocator> 
void LockFreeDeque<T, Allocator>::check_nonempty() const {
  if (size_.load() == 0) {
    throw std::out_of_range("Deque: cannot access element in empty deque");
  }
}
}
#endif
