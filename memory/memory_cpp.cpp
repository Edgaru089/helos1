#include "memory.h"
#include "memory.hpp"

#include "../extlib/dlmalloc/malloc-2.8.6.h"

void *operator new(size_t size) { return kMalloc(size); }
void *operator new[](size_t size) { return kMalloc(size); }

void operator delete(void *ptr) noexcept { kFree(ptr); }
void operator delete[](void *ptr) noexcept { kFree(ptr); }


#if __cplusplus >= 201703L
void *operator new(size_t size, std::align_val_t align) { return dlmemalign((size_t)align, size); }
void *operator new[](size_t size, std::align_val_t align) { return dlmemalign((size_t)align, size); }
#endif

#if __cplusplus >= 201402L
void operator delete(void *ptr, size_t size) noexcept { kFree(ptr); }
void operator delete[](void *ptr, size_t size) noexcept { kFree(ptr); }
#endif
#if __cplusplus >= 201703L
void operator delete(void *ptr, std::align_val_t align) noexcept { kFree(ptr); }
void operator delete[](void *ptr, std::align_val_t align) noexcept { kFree(ptr); }
void operator delete(void *ptr, size_t size, std::align_val_t align) noexcept { kFree(ptr); }
void operator delete[](void *ptr, size_t size, std::align_val_t align) noexcept { kFree(ptr); }
#endif
