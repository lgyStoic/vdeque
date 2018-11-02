template <typename T>
class DequeIterator {
public:
  DequeIterator(T*, size_t, size_t, size_t);

  T& operator*();
  void operator++();
  bool operator!=(const DequeIterator<T>&) const;

private:
  T* container_;
  size_t capacity_;
  size_t size_;
  size_t index_;
};

template <typename T> DequeIterator<T>::DequeIterator(T* container,
    size_t capacity, size_t size, size_t index)
    : container_(container), capacity_(capacity), size_(size), index_(index) {}

template <typename T> T& DequeIterator<T>::operator*() {
  return container_[index_];
}

template <typename T> void DequeIterator<T>::operator++() {
  index_ = (index_ + 1) % capacity_;
}

template <typename T> bool DequeIterator<T>::operator!=(
    const DequeIterator<T>& it) const {
  return container_ != it.container_ || capacity_ != it.capacity_
    || size_ != it.size_ || index_ != it.index_;
}
