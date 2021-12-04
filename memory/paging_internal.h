#pragma once

#include "memory.h"
#include "stdbool.h"


#ifdef __cplusplus
extern "C" {
#endif


// defined in paging_init.c
extern uint64_t paging_TotalBytes, paging_UsableBytes;
extern bool     paging_SupportExecuteDisable;
extern uint64_t paging_EndPhysicalAddress;  // past-the-end marker (and length) for physical memory
extern int      paging_EndPhysicalPage;     // past-the-end for physical pages (EndPhysicalAddress/SYSTEM_PAGE_SIZE)
extern uint64_t paging_PML4Table[512];      // Kernel-mode virtual memory paging directory pointer table (PAE/Level 4 paging)
extern uint64_t paging_LoaderCodeAddress;   // physical address for loader code section
extern int      paging_LoaderCodePageCount; // page count for loader code section
extern uint64_t paging_IdentityOffset;      // offset of memory identity mapping

// defined in paging_physical.c
#define BITMAP_BITS 64
extern uint64_t paging_physical_Bitmap[MAX_SYSTEM_MEMORY_PAGES / BITMAP_BITS]; // mapped with Bitmap[i/64] | 1<<(i%64), unlike convention
void            paging_physical_BitmapWriteOne(int begin, int end);
void            paging_physical_BitmapWriteZero(int begin, int end);
// these functions do not do any bookkeeping so use with care
uint64_t paging_physical_AllocateOneFrame();                                 // zeros the returned page
uint64_t paging_physical_AllocateOneFrame2M();                               // zeros the returned page; this is 512 normal frames
int      paging_physical_AllocateFrames(int pageCount, uint64_t frames[]);   // allocate frames, not continuous, ret allloced cnt
int      paging_physical_AllocateFrames2M(int pageCount, uint64_t frames[]); // allocate 2M frames, not continuous
void     paging_physical_FreeFrame(uint64_t frame, int pageCount);           // frees continuous frames in physical addr

// defined in paging_map.c
#define PML_PRESENT            (1ull << 0)
#define PML_WRITEABLE          (1ull << 1)
#define PML_USER               (1ull << 2)
#define PML_PAGE_WRITETHROUGH  (1ull << 3)
#define PML_PAGE_CACHE_DISABLE (1ull << 4)
#define PML_ACCESSED           (1ull << 5)
#define PML_DIRTY              (1ull << 6)
#define PML_PAGE_SIZE          (1ull << 7)
#define PML_GLOBAL             (1ull << 8)
#define PML_EXECUTE_DISABLE    (1ull << 63)
#define PML_ADDR_MASK          (0xFFFFFFFFFF000ull) // 51:12 at max length
#define PML_ADDR_MASK_2M       (0xFFFFFFFE00000ull) // 51:21 at max length
#define PML_ADDR_MASK_1G       (0xFFFFFC0000000ull) // 51:30 at max length
void paging_map_Page(uint64_t physical, uint64_t virt, int pageCount, int protectionFlags);
void paging_map_Page2M(uint64_t physical, uint64_t virt, int pageCount, int protectionFlags);
void paging_map_Page1G(uint64_t physical, uint64_t virt, int pageCount, int protectionFlags);
// Allocates pageCount fresh new 4K pages with paging_physical and maps them continuously to virtual (zeroed)
void paging_map_PageAllocated(uint64_t virt, int pageCount, int protectionFlags);
// Allocates pageCount fresh new 2M pages with paging_physical and maps them continuously to virtual (zeroed)
void paging_map_PageAllocated2M(uint64_t virt, int pageCount, int protectionFlags);
// Unmaps the pages at virtual and free the underlying physical frames, with past-the-end of the memory addr
void paging_map_FreeAllocated(uint64_t virt, uint64_t virt_end);


// defined in paging_modeswitch.S
FASTCALL_ABI void paging_modeswitch_4LevelPaging(void *pml4, int pcid);
FASTCALL_ABI void paging_modeswitch_4LevelPagingNX(void *pml4, int pcid); // with setting the Execute-Disalbe bit
FASTCALL_ABI void paging_modeswitch_Table(void *pml, int pcid);


inline static uint64_t roundUpTo2Exponent(uint64_t v) {
	uint64_t s = 1;
	while (s < v)
		s <<= 1;
	return s;
}

inline static uint64_t takeBitfield(uint64_t v, int high, int low) {
	return (v >> low) & ((1 << (high - low + 1)) - 1);
}

inline static void flush_tlb_single(uint64_t addr) {
	asm volatile(
		"invlpg (%0)" ::"r"(addr)
		: "memory");
}

inline static int roundUpToPageCount(uint64_t size) {
	if (size % SYSTEM_PAGE_SIZE == 0)
		return size / SYSTEM_PAGE_SIZE;
	return size / SYSTEM_PAGE_SIZE + 1;
}
inline static int roundUpToPageCount2M(uint64_t size) {
	if (size % SYSTEM_PAGE_2M_SIZE == 0)
		return size / SYSTEM_PAGE_2M_SIZE;
	return size / SYSTEM_PAGE_2M_SIZE + 1;
}
inline static int roundUpToPageCount1G(uint64_t size) {
	if (size % SYSTEM_PAGE_1G_SIZE == 0)
		return size / SYSTEM_PAGE_1G_SIZE;
	return size / SYSTEM_PAGE_1G_SIZE + 1;
}


#ifdef __cplusplus
} // extern "C"
#endif
