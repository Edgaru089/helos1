
#include "memory.h"
#include "paging_internal.h"
#include "../runtime/stdio.h"
#include "../interrupt/interrupt.h"
#include <stdint.h>

#include "../extlib/liballoc/liballoc_1_1.h"


int liballoc_lock() {
	if (interrupt_Enabled)
		asm volatile("cli");
	return 0;
}

int liballoc_unlock() {
	if (interrupt_Enabled)
		asm volatile("sti");
	return 0;
}


static uint64_t heapBreak = KERNEL_HEAP_VIRTUAL;

void *liballoc_alloc(size_t pages) {
	void *ret = (void *)heapBreak;
	heapBreak += SYSTEM_PAGE_SIZE * pages;

	paging_map_PageAllocated((uint64_t)ret, pages, MAP_PROT_READ | MAP_PROT_WRITE);

	io_Printf("liballoc_alloc: allocated %u pages at HEAP+%llx (%llx)\n", pages, ret - KERNEL_HEAP_VIRTUAL, ret);

	return ret;
}

int liballoc_free(void *ptr, size_t pages) {
	paging_map_FreeAllocated((uint64_t)ptr, (uint64_t)ptr + SYSTEM_PAGE_SIZE * pages);
	io_Printf("liballoc_free: freed %u pages at HEAP+%llx (%llx)\n", pages, ptr - KERNEL_HEAP_VIRTUAL, ptr);
	return 0;
}
