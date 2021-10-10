#include "memory.hpp"

void *operator new(std::size_t size) {
	void *data = kMalloc(size);
	/*if (!data) {
		throw helos::bad_alloc();
	}*/
	return data;
}

void operator delete(void *ptr) noexcept {
	kFree(ptr);
}
