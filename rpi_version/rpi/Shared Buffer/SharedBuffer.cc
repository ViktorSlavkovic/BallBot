#include "SharedBuffer"

void SharedBuffer::Push(T elem) {
	std::unique_lock<std::mutex> mlock(mutex_);
	
	if (queue_.size() == max_number_of_elems_) {
		queue_.pop();
	}

	queue_.push(elem);
	cond_.notify_one();
}

T SharedBuffer::Pop() {
	std::unique_lock<std::mutex> mlock(mutex_);

	while (queue_.empty()) {
		cond_.wait(mlock);	
	}

	auto result = queue_.front();
	queue_.pop();

	return result;
}

bool SharedBuffer::Try_pop(T& elem) {
	std::unique_lock<std::mutex> mlock(mutex_);

	if (queue_.empty()) {
		return false;
	}

	elem = queue_.front();
	queue_.pop();

	return true;
};

int SharedBuffer::Size() {
	std::unique_lock<std::mutex> mlock(mutex_);
	
	return queue_.size();
}


bool SharedBuffer::Empty() {
	std::unique_lock<std::mutex> mlock(mutex_);

	return queue_.empty();
}
