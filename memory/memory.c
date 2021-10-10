
#include "../main.h"
#include "memory.h"
#include "../runtime/stdio.h"
#include "../extlib/liballoc/liballoc_1_1.h"

#include <string.h>


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
	void *mem = liballoc_kmalloc(size);
	io_Printf("kMalloc: size=%llu, pos=0x%llx\n", size, mem);
	return mem;
}

void kFree(void *data) {
	io_Printf("kFree: 0x%llx\n", data);
	liballoc_kfree(data);
}

void *kMemoryMap(void *desiredVirtual, int pageCount, int protectionFlags, int flags, int fd) {
}

void kMemoryUnmap(void *pageStart, int pageCount) {
}
