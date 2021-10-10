
#include "memory.h"
#include "paging_internal.h"
#include "../runtime/panic_assert.h"
#include "string.h"


// some learning:
// https://forum.osdev.org/viewtopic.php?f=1&t=51392 (Confirmative question about 4-level and 5-level paging)

void paging_map_Page(uint64_t physical, uint64_t virtual, int pageCount, int protectionFlags) {
	assert(physical % SYSTEM_PAGE_SIZE == 0 && "Physical address not page-aligned");
	assert(virtual % SYSTEM_PAGE_SIZE == 0 && "Virtual address not page-aligned");

	while (pageCount--) {
		// PML4E index pointing to a PML3 table (Page-Directory-Pointer Table)
		uint64_t *table = paging_PML4Table;
		uint64_t  i     = takeBitfield(virtual, 47, 39);
		if ((table[i] & PML_PRESENT) == 0) // allocate a new page as PML3 table
			table[i] = paging_physical_AllocateOneFrame() | PML_PRESENT | PML_WRITEABLE;

		// PML3E(PDPTE) index pointing to a PML2 table (Page-Directory)
		table = (uint64_t *)(table[i] & PML_ADDR_MASK);
		i     = takeBitfield(virtual, 38, 30);
		if ((table[i] & PML_PRESENT) == 0) // allocate page as page directory
			table[i] = paging_physical_AllocateOneFrame() | PML_PRESENT | PML_WRITEABLE;

		// PML2E(PD) index pointing to a PML1 table (Page Table)
		table = (uint64_t *)(table[i] & PML_ADDR_MASK);
		i     = takeBitfield(virtual, 29, 21);
		if ((table[i] & PML_PRESENT) == 0) // allocate page as page table
			table[i] = paging_physical_AllocateOneFrame() | PML_PRESENT | PML_WRITEABLE;

		// Finally, the page table.
		table = (uint64_t *)(table[i] & PML_ADDR_MASK);
		i     = takeBitfield(virtual, 20, 12);

		table[i] = physical | PML_PRESENT |
				   ((protectionFlags & MAP_PROT_WRITE) ? PML_WRITEABLE : 0) |
				   ((paging_SupportExecuteDisable && !(protectionFlags & MAP_PROT_EXEC)) ? PML_EXECUTE_DISABLE : 0);


		flush_tlb_single(virtual);
		physical += SYSTEM_PAGE_SIZE;
		virtual += SYSTEM_PAGE_SIZE;
	}
}

void paging_map_Page2M(uint64_t physical, uint64_t virtual, int pageCount, int protectionFlags) {
	assert(physical % SYSTEM_PAGE_2M_SIZE == 0 && "Physical address not page-aligned");
	assert(virtual % SYSTEM_PAGE_2M_SIZE == 0 && "Virtual address not page-aligned");

	while (pageCount--) {
		// PML4E index pointing to a PML3 table (Page-Directory-Pointer Table)
		uint64_t *table = paging_PML4Table;
		uint64_t  i     = takeBitfield(virtual, 47, 39);
		if ((table[i] & PML_PRESENT) == 0) // allocate a new page as PML3 table
			table[i] = paging_physical_AllocateOneFrame() | PML_PRESENT | PML_WRITEABLE;

		// PML3E(PDPTE) index pointing to a PML2 table (Page-Directory)
		table = (uint64_t *)(table[i] & PML_ADDR_MASK);
		i     = takeBitfield(virtual, 38, 30);
		if ((table[i] & PML_PRESENT) == 0) // allocate page as page directory
			table[i] = paging_physical_AllocateOneFrame() | PML_PRESENT | PML_WRITEABLE;

		// PML2E(PD) index pointing to a PML1 table (Page Table)
		table = (uint64_t *)(table[i] & PML_ADDR_MASK);
		i     = takeBitfield(virtual, 29, 21);
		if ((table[i] & PML_PRESENT) != 0 && (table[i] & PML_PAGE_SIZE) == 0) // deallocate the page if present
			paging_physical_FreeFrame(table[i] & PML_ADDR_MASK, 1);

		// 2MB pages in Page Tables
		table[i] = physical | PML_PRESENT | PML_PAGE_SIZE |
				   ((protectionFlags & MAP_PROT_WRITE) ? PML_WRITEABLE : 0) |
				   ((paging_SupportExecuteDisable && !(protectionFlags & MAP_PROT_EXEC)) ? PML_EXECUTE_DISABLE : 0);


		flush_tlb_single(virtual);
		physical += SYSTEM_PAGE_2M_SIZE;
		virtual += SYSTEM_PAGE_2M_SIZE;
	}
}

void paging_map_Page1G(uint64_t physical, uint64_t virtual, int pageCount, int protectionFlags) {
	assert(physical % SYSTEM_PAGE_1G_SIZE == 0 && "Physical address not page-aligned");
	assert(virtual % SYSTEM_PAGE_1G_SIZE == 0 && "Virtual address not page-aligned");

	while (pageCount--) {
		// PML4E index pointing to a PML3 table (Page-Directory-Pointer Table)
		uint64_t *table = paging_PML4Table;
		uint64_t  i     = takeBitfield(virtual, 47, 39);
		if ((table[i] & PML_PRESENT) == 0) // allocate a new page as PML3 table
			table[i] = paging_physical_AllocateOneFrame() | PML_PRESENT | PML_WRITEABLE;

		// PML3E(PDPTE) index pointing to a PML2 table (Page-Directory)
		table = (uint64_t *)(table[i] & PML_ADDR_MASK);
		i     = takeBitfield(virtual, 38, 30);
		if ((table[i] & PML_PRESENT) != 0 && (table[i] & PML_PAGE_SIZE) == 0) // deallocate the page if present
			paging_physical_FreeFrame(table[i] & PML_ADDR_MASK, 1);

		// 1GB pages in Page Directories
		table[i] = physical | PML_PRESENT | PML_PAGE_SIZE |
				   ((protectionFlags & MAP_PROT_WRITE) ? PML_WRITEABLE : 0) |
				   ((paging_SupportExecuteDisable && !(protectionFlags & MAP_PROT_EXEC)) ? PML_EXECUTE_DISABLE : 0);


		flush_tlb_single(virtual);
		physical += SYSTEM_PAGE_1G_SIZE;
		virtual += SYSTEM_PAGE_1G_SIZE;
	}
}

void paging_map_PageAllocated(uint64_t virtual, int pageCount, int protectionFlags) {
	assert(virtual % SYSTEM_PAGE_SIZE == 0 && "Virtual address not page-aligned");

	// skip the first 1M
	int i = 1024 / 4 / BITMAP_BITS;
	while (pageCount--) {
		uint64_t freshPage = 0;

		// this code is in sync with paging_physical.c, paging_physical_AllocateOneFrame, so you have to modify both
		for (; i < paging_EndPhysicalPage / BITMAP_BITS; i++)
			if (paging_physical_Bitmap[i] != ~0ull)
				for (int j = 0; j < BITMAP_BITS; j++)
					if ((paging_physical_Bitmap[i] & (1ull << j)) == 0) {
						paging_physical_Bitmap[i] |= (1ull << j);
						freshPage = (((uint64_t)i) * BITMAP_BITS + j) * SYSTEM_PAGE_SIZE;
						memset((void *)freshPage, 0, SYSTEM_PAGE_SIZE);
					}

		paging_map_Page(freshPage, virtual, 1, protectionFlags);
		virtual += SYSTEM_PAGE_SIZE;
	}
}

void paging_map_PageAllocated2M(uint64_t virtual, int pageCount, int protectionFlags) {
	assert(pageCount > HELOS_BUFFER_SIZE / SYSTEM_PAGE_2M_SIZE * 8 && "helos_Buffer unable to hold all pointers");
	assert(virtual % SYSTEM_PAGE_2M_SIZE == 0 && "Virtual address not page-aligned");

	uint64_t *buf = (uint64_t *)Buffer;

	int allocated = paging_physical_AllocateFrames2M(pageCount, buf);
	for (int i = 0; i < allocated; i++)
		paging_map_Page2M(buf[i], virtual + SYSTEM_PAGE_2M_SIZE * i, 1, protectionFlags);
}

void paging_map_FreeAllocated(uint64_t virtual, uint64_t end) {
	assert(virtual % SYSTEM_PAGE_SIZE == 0 && "Virtual address not page-aligned");

	while (virtual < end) {
		uint64_t *table = paging_PML4Table;
		uint64_t  i     = takeBitfield(virtual, 47, 39);
		if ((table[i] & PML_PRESENT) == 0)
			goto loop_end;

		table = (uint64_t *)(table[i] & PML_ADDR_MASK);
		i     = takeBitfield(virtual, 38, 30);
		if ((table[i] & PML_PRESENT) == 0)
			goto loop_end;
		if (table[i] & PML_PAGE_SIZE) { // 1G mapping
			paging_physical_FreeFrame(table[i] & PML_ADDR_MASK_1G, 512 * 512);
			table[i] = 0;
			virtual += SYSTEM_PAGE_1G_SIZE;
			continue;
		}

		table = (uint64_t *)(table[i] & PML_ADDR_MASK);
		i     = takeBitfield(virtual, 29, 21);
		if ((table[i] & PML_PRESENT) == 0)
			goto loop_end;
		if (table[i] & PML_PAGE_SIZE) { // 2M mapping
			paging_physical_FreeFrame(table[i] & PML_ADDR_MASK_2M, 512);
			table[i] = 0;
			virtual += SYSTEM_PAGE_2M_SIZE;
			continue;
		}

		table = (uint64_t *)(table[i] & PML_ADDR_MASK);
		i     = takeBitfield(virtual, 20, 12);

		if (table[i] & PML_PRESENT) {
			paging_physical_FreeFrame(table[i] & PML_ADDR_MASK, 1);
			table[i] = 0;
		}

loop_end:
		virtual += SYSTEM_PAGE_SIZE;
	}
}
