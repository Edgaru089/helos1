
#include "heap_break.h"
#include "memory.h"
#include "paging_internal.h"
#include "../runtime/stdio.h"


static uintptr_t heapBreak = KERNEL_HEAP_VIRTUAL, pageBreak = KERNEL_HEAP_VIRTUAL;

void *memory_AddBreak(intptr_t inc) {
	if (heapBreak + inc < KERNEL_HEAP_VIRTUAL)
		return (void *)-1;

	heapBreak += inc;

	if (heapBreak > pageBreak) {
		// we need more pages
		int pageCount = roundUpToPageCount(heapBreak - pageBreak);
		io_Printf("memory_AddBreak(): 0x%llx -> 0x%llx, add %d, allocating %d pages\n", heapBreak - inc, heapBreak, inc, pageCount);
		paging_map_PageAllocated(pageBreak, pageCount, MAP_PROT_READ | MAP_PROT_WRITE);
		pageBreak += SYSTEM_PAGE_SIZE * pageCount;
	} else
		io_Printf("memory_AddBreak(): 0x%llx -> 0x%llx, add %d\n", heapBreak - inc, heapBreak, inc);

	return (void *)(heapBreak - inc);
}
