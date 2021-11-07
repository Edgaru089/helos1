
#include "../main.h"
#include "../efimain.h"
#include "memory.h"
#include "../runtime/stdio.h"
#include "../extlib/dlmalloc/malloc-2.8.6.h"

#include <string.h>


uint64_t memory_KernelMappingBottom = KERNEL_MISC_MAPPING;

void *efiMallocTyped(size_t size, EFI_MEMORY_TYPE type) {
	void *data;
	efiBootServices->AllocatePool(type, size, &data);
	memset(data, 0, size);
	return data;
}

void *efiMalloc(size_t size) {
	return efiMallocTyped(size, EfiLoaderData);
}

void efiFree(void *data) {
	efiBootServices->FreePool(data);
}

void *kMalloc(size_t size) {
	void *mem = dlmalloc(size);
	return mem;
}

void kFree(void *data) {
	dlfree(data);
}

void *kMemoryMap(void *desiredVirtual, int pageCount, int protectionFlags, int flags, int fd) {
}

void kMemoryUnmap(void *pageStart, int pageCount) {
}
