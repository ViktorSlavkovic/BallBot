#include "SharedBuffer"

void SharedBuffer::push(T* elem) {
	std::unique_lock<std::mutex> mlock(mutex_);
		
	if (elem == nullptr) {
		return;
	}

	if (queue_.size() == max_number_of_elems_) {
		queue_.pop();
	}

	queue_.push(elem);
	cond_.notify_one();
}

T* SharedBuffer::pop() {
	std::unique_lock<std::mutex> mlock(mutex_);

	while (queue_.empty()) {
		cond_.wait(mlock);	
	}

	T* result = queue_.front();
	queue_.pop();

	return result;
}

int SharedBuffer::size() {
	std::unique_lock<std::mutex> mlock(mutex_);
	
	return queue_.size();
}


bool SharedBuffer::empty() {
	std::unique_lock<std::mutex> mlock(mutex_);

	return queue_.empty();
}
