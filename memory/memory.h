#pragma once

#include "../main.h"
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

#define KERNEL_CODE_VIRTUAL        0xFFFFFFFFC0000000ull // 2^64 - 1GiB
#define KERNEL_STACK_END_VIRTUAL   (KERNEL_CODE_VIRTUAL) // kernel stack sits right below kernel code
#define KERNEL_STACK_INITIAL_SIZE  SYSTEM_PAGE_2M_SIZE   // currently must be 2 MiB
#define KERNEL_HEAP_VIRTUAL        0xFFFFFFFF00000000ull // 2^64 - 4GiB
#define KERNEL_FRAMEBUFFER_MAPPING 0xFFFFFFFEE0000000ull // 2^64 - 4GiB - 512MiB
#define KERNEL_MISC_MAPPING        0xFFFFFFFEC0000000ull // 2^64 - 5GiB

#define KERNEL_IDT_MAPPING KERNEL_MISC_MAPPING
#define KERNEL_IDT_SIZE    (256ull * 16) // fill the 256 interrupt vectors

#define KERNEL_GDT_MAPPING (KERNEL_MISC_MAPPING + KERNEL_IDT_SIZE)
#define KERNEL_GDT_SIZE    (16ull * 8)

#define KERNEL_MISC_NEXT (KERNEL_MISC_MAPPING + KERNEL_IDT_SIZE + KERNEL_GDT_SIZE)

#define KERNEL_MISC_SIZE (KERNEL_IDT_SIZE + KERNEL_GDT_SIZE) // add all the misc sizes


extern uint64_t        paging_LoaderCodeAddress, paging_LoaderCodeSize; // physical address for loader code section
static inline uint64_t paging_MapFunction(void *func) {
	return ((uint64_t)func - paging_LoaderCodeAddress) + KERNEL_CODE_VIRTUAL;
}


// efiMallocTyped allocates from EFI_BOOT_SERVICES.AllocatePool.
void *efiMallocTyped(size_t size, EFI_MEMORY_TYPE type);

// efiMallocTyped allocates from EFI_BOOT_SERVICES.AllocatePool
// with a memory type of EfiLoaderData.
void *efiMalloc(size_t size);

// efiFree frees data allocated from efiMalloc.
void efiFree(void *data);


// kMalloc allocates from system memory directly after paging has been set up
void *kMalloc(size_t size);

// kFree frees data allocated from kMalloc.
void kFree(void *data);


extern EFI_MEMORY_DESCRIPTOR *efiMemoryMap;
extern UINTN                  efiMemoryMapSize;
extern UINTN                  efiMemoryMapKey;
extern UINTN                  efiDescriptorSize;
extern UINT32                 efiDescriptorVertion;

// runtime_InitPaging initializes paging and kMalloc/kFree allocator.
// This function calls ExitBootServices()!!! which is great
// Furthermore, it sets up a new stack, calls kMain() and does not return.
//
// If it fails, Panic() is called.
noreturn void runtime_InitPaging();

#define SYSTEM_PAGE_SIZE        4096ull       // UEFI uses 4KiB pages by default
#define SYSTEM_PAGE_2M_SIZE     2097152ull    // 2 MiB page size
#define SYSTEM_PAGE_1G_SIZE     1073741824ull // 1 GiB page size
#define MAX_SYSTEM_MEMORY_PAGES 16777216ull   // 64 GiB


#define MAP_PROT_NONE  0
#define MAP_PROT_EXEC  1
#define MAP_PROT_WRITE 2
#define MAP_PROT_READ  4

#define MAP_DATA             1 // Data, not initialized (kept as-is)
#define MAP_INITIALIZED_DATA 2 // Data, zeroed
#define MAP_FILE             3 // Memory-mapped file IO

// kMemoryPageSize returns the size, in bytes, of the physical memory page.
inline static size_t kMemoryPageSize() {
	return SYSTEM_PAGE_SIZE;
}

// kMemoryMap maps new physical pages into the kernel virtual memory space.
//
// The memory is not cleared.
void *kMemoryMap(void *desiredVirtual, int pageCount, int protectionFlags, int dataType, int fd);

// kMemoryUnmap unmaps previously mapped physical memory of the kernel space.
void kMemoryUnmap(void *pageStart, int pageCount);


#ifdef __cplusplus
} // extern "C"
#endif
