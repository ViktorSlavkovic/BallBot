#ifndef SHARED_BUFFER_H
#define SHARED_BUFFER_H

#include <mutex>
#include <queue>
#include <condition_variable>

template <class T>
class SharedBuffer {
	unsigned int max_number_of_elems_;

	std::queue<T> queue_;
	std::mutex mutex_;
	std::condition_variable cond_;

public:

	SharedBuffer(int max_number_of_elems = 100) : max_number_of_elems_(max_number_of_elems) {}

	void Push(T elem) {
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

	int Size() {
		std::unique_lock<std::mutex> mlock(mutex_);
		
		return queue_.size();
	}

	bool Empty() {
		std::unique_lock<std::mutex> mlock(mutex_);

		return queue_.empty();
	}

	// Tries to pop an element, if there is no new element,
	// reference to T is not changed. Non-blocking call.
	bool Try_pop(T& t) {
		std::unique_lock<std::mutex> mlock(mutex_);

		if (queue_.empty()) {
			return false;
		}

		auto elem = queue_.front();
		queue_.pop();

		return true;
	};

};


#endif