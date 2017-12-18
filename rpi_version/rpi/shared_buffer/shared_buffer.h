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

	void Push(T elem);
	
	// Pops an element from queue. Blocking call.
	T Pop();

	int Size();
	bool Empty();

	// Tries to pop an element, if there is no new element,
	// reference to T is not changed. Non-blocking call.
	bool Try_pop(T& t);

};

#endif