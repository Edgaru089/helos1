
#include "vterm.hpp"

static void *defaultMalloc(size_t size, void *userdata) {
	void *ptr = kMalloc(size);
	return ptr;
}

static void defaultFree(void *ptr, void *userdata) {
	kFree(ptr);
}

VTermAllocatorFunctions vterm_Allocator = {defaultMalloc, defaultFree};
