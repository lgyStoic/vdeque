#ifndef _FDT_DEQUE_H_
#define _FDT_DEQUE_H_
#include "DequeIterator.h"

#include <string>
#include <ostream>
#include <sstream>
#include <initializer_list>
#include <stdexcept>
#include <memory>
#include <iostream>

namespace fdt {
template<typename T> class DequeIterator;
template<typename T,  class Allocator = std::allocator<T> >
class Deque {
public:
  Deque();
  Deque(size_t capacity, const Allocator& alloca = Allocator());
  Deque(const Deque<T>& deque, const Allocator& alloca = Allocator());
  Deque(std::initializer_list<T> container, const Allocator& alloca = Allocator());
  ~Deque();
  Deque& operator=(const Deque& deque);
  Deque& operator=(std::initializer_list<T> container);

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
  friend std::ostream& operator<<(std::ostream& out, const Deque<U>& deque);

private:
  T* container_;
  Allocator alloca_;
  size_t capacity_;
  size_t front_;
  size_t size_;

  static const size_t DEFAULT_CAPACITY = 64;

  void reallocate();
  void shift_left(size_t, size_t, size_t);
  void shift_right(size_t, size_t, size_t);
  void out_of_range(const char*, size_t, const char*, const char*, size_t) const;
  void check_nonempty() const;
};

template <typename T, class Allocator> 
Deque<T, Allocator>::Deque() : Deque(64) {}

template <typename T, class Allocator> 
Deque<T, Allocator>::Deque(size_t capacity, const Allocator& alloca)
    : alloca_(alloca), capacity_(capacity), size_(0), front_(0) {
  container_ = alloca_.allocate(capacity_);
}

template <typename T, class Allocator> 
Deque<T, Allocator>::Deque(const Deque<T>& deque, const Allocator& alloca)
    : Deque(deque.capacity_, alloca) {
  size_t i = 0;
  for (const T& value : deque) {
    container_[i++] = value;
  }
  size_ = deque.size_;
}

template <typename T, class Allocator> 
Deque<T, Allocator>::Deque(std::initializer_list<T> container, const Allocator &alloca)
    : Deque(container.size() * 2, alloca) {
  size_t i = 0;
  for (const T& value : container) {
    container_[i++] = value;
  }
  size_ = container.size();
}

template <typename T, class Allocator> 
Deque<T, Allocator>::~Deque() {
  alloca_.deallocate(container_, capacity_);
}

template <typename T, class Allocator> 
Deque<T, Allocator>& Deque<T, Allocator>::operator=(const Deque<T, Allocator>& deque) {
  size_ = deque.size_;
  front_ = 0;
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
Deque<T, Allocator>& Deque<T, Allocator>::operator=(std::initializer_list<T> container) {
  size_ = container.size();
  front_ = 0;
  if (capacity_ < size_) {
    alloca_.deallocate(container_, capacity_);
    capacity_ = size_ * 2;
    alloca_.allocate(capacity_);
  }
  size_t i = 0;
  for (const T& value : container) {
    container_[i++] = value;
  }
  return *this;
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::push_front(T value) {
  front_ = (front_ + capacity_- 1) % capacity_;
  container_[front_] = value;
  size_++;
  reallocate();
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::push_back(T value) {
  container_[(front_ + size_) % capacity_] = value;
  size_++;
  reallocate();
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::pop_front() {
  check_nonempty();
  size_--;
  front_ = (front_ + 1) % capacity_;
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::pop_back() {
  check_nonempty();
  size_--;
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::erase(const DequeIterator<T>& begin, const DequeIterator<T>& end) {
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
  if (begin.index_ + end.index_ < size_) {
    shift_right(0, begin.index_, offset);
    front_ = (front_ + offset) % capacity_;
  }
  else {
    shift_left(end.index_, size_, offset);
  }
  size_ -= offset;
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::erase(const DequeIterator<T>& it) {
  return erase(it, it + 1);
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::insert(const DequeIterator<T>& it, T value) {
  std::cout << "t idx:" << it.index_ << std::endl;
  if (it.index_ > size_) {
    out_of_range("it.index_", it.index_, ">", "this->size()", size_);
  }
  if (it.index_ < size_ / 2) {
    shift_left(0, it.index_, 1);
    front_ = (front_ + capacity_ - 1) % capacity_;
  }
  else {
    shift_right(it.index_, size_, 1);
  }
  std::cout << "front " << front_ << std::endl;
  container_[(it.index_ + front_) % capacity_] = value;
  size_++;
  reallocate();
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::reserve(size_t capacity) {
  if (capacity <= capacity_) {
    return;
  }
  T* new_container = alloca_.allocate(capacity);
  for (size_t i = 0; i < size_; i++) {
    new_container[i] = container_[(i + front_) % capacity_];
  }
  alloca_.deallocate(container_, capacity_);
  container_ = new_container;
  capacity_ = capacity;
  front_ = 0;
}

template <typename T, class Allocator>
void Deque<T, Allocator>::resize(size_t size, T value) {
  if (size > capacity_) {
    reserve(size);
  }
  for (size_t i = front_ + size_; i < front_ + size; i++) {
    container_[i % capacity_] = value;
  }
  size_ = size;
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::clear() {
  size_ = 0;
  front_ = 0;
}

template <typename T, class Allocator> 
T& Deque<T, Allocator>::front() {
  check_nonempty();
  return container_[front_];
}

template <typename T, class Allocator> 
T& Deque<T, Allocator>::back() {
  check_nonempty();
  return container_[(front_ + size_ - 1) % capacity_];
}

template <typename T, class Allocator> 
T& Deque<T, Allocator>::at(size_t index) {
  if (index >= size_) {
    out_of_range("index", index, ">=", "this->size()", size_);
  }
  return operator[](index);
}

template <typename T, class Allocator> 
T& Deque<T, Allocator>::operator[](size_t index) {
  return container_[(front_ + index) % capacity_];
}

template <typename T, class Allocator> 
T Deque<T, Allocator>::front() const {
  return front();
}

template <typename T, class Allocator> 
T Deque<T, Allocator>::back() const {
  return back();
}

template <typename T, class Allocator> 
T Deque<T, Allocator>::at(size_t index) const {
  return at(index);
}

template <typename T, class Allocator> 
T Deque<T, Allocator>::operator[](size_t index) const {
  return operator[](index);
}

template <typename T, class Allocator> 
DequeIterator<T> Deque<T,  Allocator>::begin() const {
  return DequeIterator<T>(container_, capacity_, size_, front_, 0);
}

template <typename T, class Allocator> 
DequeIterator<T> Deque<T, Allocator>::end() const {
  return DequeIterator<T>(container_, capacity_, size_, front_, size_);
}

template <typename T, class Allocator> 
size_t Deque<T, Allocator>::capacity() const {
  return capacity_;
}

template <typename T, class Allocator> 
size_t Deque<T, Allocator>::size() const {
  return size_;
}

template <typename T, class Allocator> 
bool Deque<T, Allocator>::empty() const {
  return size_ == 0;
}

template <typename T, class Allocator>
std::string Deque<T, Allocator>::to_string() const {
  std::ostringstream out;
  out << "[ ";
  for (const T& value : *this) {
    out << value << " ";
  }
  out << "]";
  return out.str();
}

template <typename T> 
std::ostream& operator<<(std::ostream& out,
    const Deque<T>& deque) {
  return out << deque.to_string();
}

template <typename T, class Allocator> 
inline void Deque<T, Allocator>::reallocate() {
  if (size_ < capacity_) {
    return;
  }
  reserve(capacity_ * 2);
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::shift_left(size_t begin, size_t end, size_t offset) {
  for (size_t i = begin + front_; i < end + front_; i++) {
    container_[(i - offset) % capacity_] = container_[i % capacity_];
  }
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::shift_right(size_t begin, size_t end, size_t offset) {
  for (size_t i = end + front_ - 1; i >= begin + front_; i--) {
    container_[(i + offset) % capacity_] = container_[i % capacity_];
  }
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::out_of_range(const char* id_1, size_t value_1,
    const char* op, const char* id_2, size_t value_2) const {
  std::ostringstream out;
  out << "Deque: " << id_1 << " (which is " << value_1 << ") "
    << op << " " << id_2 << " (which is " << value_2 << ")";
  throw std::out_of_range(out.str());
}

template <typename T, class Allocator> 
void Deque<T, Allocator>::check_nonempty() const {
  if (size_ == 0) {
    throw std::out_of_range("Deque: cannot access element in empty deque");
  }
}
}
#endif