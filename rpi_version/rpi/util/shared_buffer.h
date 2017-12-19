#ifndef SHARED_BUFFER_H_
#define SHARED_BUFFER_H_

#include "util/macros.h"

#include <condition_variable>
#include <mutex>
#include <queue>

namespace util {

template <typename T>
class SharedBuffer {
 public:
  SharedBuffer(): SharedBuffer(100) {}
  SharedBuffer(int max_number_of_elems)
      : max_number_of_elems_(max_number_of_elems) {}
  // Pushes an element into the queue. If the queue is full, all the elements
  // are shifted (a pop is performed) in order to make place for a new element,
  // hence the push operation cannot fail.
  void Push(const T& elem) {
    std::unique_lock<std::mutex> mlock(mutex_);
    
    if (queue_.size() == max_number_of_elems_) {
      queue_.pop();
    }

    queue_.push(elem);
    cond_.notify_one();
  }
  // Pops an element from queue. Blocking call.
  T Pop() {
    std::unique_lock<std::mutex> mlock(mutex_);

    while (queue_.empty()) {
      cond_.wait(mlock);	
    }

    auto result = queue_.front();
    queue_.pop();

    return result;
  }
  // Tries to pop an element. If there is no new element, reference to T is not
  // changed. Returns true on successfull pop.
  bool TryPop(T& elem_out) {
    std::unique_lock<std::mutex> mlock(mutex_);

    if (queue_.empty()) {
      return false;
    }

    elem_out = queue_.front();
    queue_.pop();

    return true;
  }
  // Returns the queue size.
  int Size() const {
    std::unique_lock<std::mutex> mlock(mutex_);
    return queue_.size();
  }
  // Returbs true if the queue is empty.
  bool Empty() const {
    std::unique_lock<std::mutex> mlock(mutex_);
    return queue_.empty();
  }

 private:
  // Represents queue capacity.
  unsigned int max_number_of_elems_;
  // Queue container.
  std::queue<T> queue_;
  // Guards all queue operations.
  std::mutex mutex_;
  // Pop condition.
  std::condition_variable cond_;

  DISALLOW_COPY_AND_ASSIGN(SharedBuffer);
};

};  // namespace util

#endif  // SHARED_BUFFER_H_