#pragma once

#include <cstddef>
#include <limits>
#include <new>

#include "memory.h"


namespace helos {


class bad_alloc: public std::exception {
public:
	bad_alloc() noexcept {}
	bad_alloc(const bad_alloc &other) noexcept {}
	bad_alloc &operator=(const bad_alloc &other) noexcept { return *this; }

	virtual const char *what() const noexcept override { return "helos::bad_alloc"; }
};


// kAllocator is a class wrapper for kMalloc/Free satisfying the named requirement Allocator.
template<typename Type>
class kAllocator {
public:
	typedef Type value_type;

	kAllocator() = default;
	template<typename Other>
	constexpr kAllocator(const kAllocator<Other> &) {}

	Type *allocate(std::size_t n) {
		return kMalloc(n * sizeof(Type));
	}

	void deallocate(Type *p, std::size_t n) {
		kFree(p);
	}
};

template<class T, class U>
bool operator==(const kAllocator<T> &, const kAllocator<U> &) {
	return true;
}
template<class T, class U>
bool operator!=(const kAllocator<T> &, const kAllocator<U> &) {
	return false;
}

} // namespace helos


// overload new/delete only in the real kernel, not in testing
#ifdef HELOS

// globally overload the new and delete operators
// so keep this header at the top of every source file
//
// operators new and delete only call kMalloc/kFree, so C++ code
// must stay after paging setup
void *operator new(std::size_t size);
void *operator new[](std::size_t size);

#if __cplusplus >= 201703L
void *operator new(std::size_t size, std::align_val_t align);
void *operator new[](std::size_t size, std::align_val_t align);
#endif

void operator delete(void *ptr) noexcept;
void operator delete[](void *ptr) noexcept;
#if __cplusplus >= 201402L
void operator delete(void *ptr, std::size_t size) noexcept;
void operator delete[](void *ptr, std::size_t size) noexcept;
#endif
#if __cplusplus >= 201703L
void operator delete(void *ptr, std::align_val_t align) noexcept;
void operator delete[](void *ptr, std::align_val_t align) noexcept;
void operator delete(void *ptr, std::size_t size, std::align_val_t align) noexcept;
void operator delete[](void *ptr, std::size_t size, std::align_val_t align) noexcept;
#endif

#endif // HELOS
