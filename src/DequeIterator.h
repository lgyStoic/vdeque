#ifndef _FDT_DEQUE_ITERATOR_H_
#define _FDT_DEQUE_ITERATOR_H_

#include "Deque.h"
#include "LockfreeQueue.h"

namespace fdt {
template <typename T>
class DequeIterator {
public:
  DequeIterator(T* container, size_t capacity, size_t size, size_t front, size_t index);
  DequeIterator(const DequeIterator<T>& it);
  DequeIterator<T>& operator=(const DequeIterator& it);

  T& operator*();
  T& operator[](int);
  T operator*() const;
  T operator[](int) const;
  DequeIterator<T>& operator++();
  DequeIterator<T>& operator--();
  DequeIterator<T> operator++(int);
  DequeIterator<T> operator--(int);
  DequeIterator<T>& operator+=(int);
  DequeIterator<T>& operator-=(int);
  DequeIterator<T> operator+(int offfset) const;
  DequeIterator<T> operator-(int offset) const;
  int operator-(const DequeIterator<T>& it) const;
  bool operator==(const DequeIterator<T>& it) const;
  bool operator!=(const DequeIterator<T>& it) const;
  bool operator<=(const DequeIterator<T>& it) const;
  bool operator>=(const DequeIterator<T>& it) const;
  bool operator<(const DequeIterator<T>& it) const;
  bool operator>(const DequeIterator<T>& it) const;

  template <typename U>
  friend DequeIterator<U> operator+(int offset, const DequeIterator<U>& it);

private:
  T* container_;
  size_t capacity_;
  size_t size_;
  size_t front_;
  size_t index_;

  bool same_container(const DequeIterator<T>& it) const;

  template <typename, class> friend class Deque;
  template <typename, class> friend class LockFreeDeque;
};

template <typename T>
DequeIterator<T>::DequeIterator(T* container,
    size_t capacity, size_t size, size_t front, size_t index)
    : container_(container), capacity_(capacity), size_(size), front_(front),
      index_(index) {}

template <typename T>
DequeIterator<T>::DequeIterator(const DequeIterator<T>& it):DequeIterator(it.container_, it.capacity_, it.size_, it.front_, it.index_){ 
}

template <typename T> 
DequeIterator<T>& DequeIterator<T>::operator=(const DequeIterator& it) {
  container_ = it.container_;
  capacity_ = it.capacity_;
  size_ = it.size_;
  front_ = it.front_;
  index_ = it.index_;
  return *this;
}

template <typename T> 
T& DequeIterator<T>::operator*() {
  return container_[(index_ + front_) % capacity_];
}

template <typename T> 
T& DequeIterator<T>::operator[](int offset) {
  return container_[(index_ + front_ + offset) % capacity_];
}

template <typename T> 
T DequeIterator<T>::operator*() const {
  return operator*();
}

template <typename T> 
T DequeIterator<T>::operator[](int offset) const {
  return operator[](offset);
}

template <typename T> 
DequeIterator<T>& DequeIterator<T>::operator++() {
  index_++;
  return *this;
}


template <typename T> 
DequeIterator<T>& DequeIterator<T>::operator--() {
  index_--;
  return *this;
}

template <typename T> 
DequeIterator<T> DequeIterator<T>::operator++(int) {
  DequeIterator<T> temp = *this;
  index_++;
  return temp;
}

template <typename T> 
DequeIterator<T> DequeIterator<T>::operator--(int) {
  DequeIterator<T> temp = *this;
  index_--;
  return temp;
}

template <typename T>
DequeIterator<T>& DequeIterator<T>::operator+=(int offset) {
  index_ += offset;
  return *this;
}

template <typename T> 
DequeIterator<T>& DequeIterator<T>::operator-=(int offset) {
  index_ -= offset;
  return *this;
}

template <typename T> 
DequeIterator<T> DequeIterator<T>::operator+(int offset) const {
  return *this;
  // DequeIterator<T> it = *this;
  // it.index_ += offset;
  // return it;
}


template <typename T> 
DequeIterator<T> operator+(int offset, const DequeIterator<T>& it) {
  return it.operator+(offset);
}

template <typename T> 
DequeIterator<T> DequeIterator<T>::operator-(int offset) const {
  return operator+(-offset);
}

template <typename T>
int DequeIterator<T>::operator-(const DequeIterator<T>& it) const {
  return ((int) index_) - ((int) it.index_);
}

template <typename T>
bool DequeIterator<T>::operator==(const DequeIterator<T>& it) const {
  return same_container(it) && index_ == it.index_;
}

template <typename T> 
bool DequeIterator<T>::operator!=(const DequeIterator<T>& it) const {
  return !same_container(it) || index_ != it.index_;
}

template <typename T> 
bool DequeIterator<T>::operator<=(const DequeIterator<T>& it) const {
  return same_container(it) && index_ <= it.index_;
}

template <typename T> 
bool DequeIterator<T>::operator>=(const DequeIterator<T>& it) const {
  return same_container(it) && index_ >= it.index_;
}

template <typename T> 
bool DequeIterator<T>::operator<(const DequeIterator<T>& it) const {
  return same_container(it) && index_ < it.index_;
}

template <typename T> 
bool DequeIterator<T>::operator>(const DequeIterator<T>& it) const {
  return same_container(it) && index_ > it.index_;
}

template <typename T> 
bool DequeIterator<T>::same_container(const DequeIterator<T>& it) const {
  return container_ == it.container_ && capacity_ && it.capacity_
    && size_ == it.size_ && front_ == it.front_;
}
}
#endif
