
#include "memory.h"
#include "paging_internal.h"
#include "../runtime/stdio.h"
#include <string.h>

uint64_t paging_physical_Bitmap[MAX_SYSTEM_MEMORY_PAGES / BITMAP_BITS];

static inline uint64_t fillBits(int begin, int last) {
	if (last == BITMAP_BITS - 1)
		return ~((1ull << begin) - 1ull);
	if (begin == 0)
		return (1ull << (last + 1ull)) - 1ull;
	return (~((1ull << begin) - 1ull)) & ((1ull << (last + 1ull)) - 1ull);
}

void paging_physical_BitmapWriteOne(int begin, int end) {
	int whereBegin = begin / BITMAP_BITS, whereLast = (end - 1) / BITMAP_BITS;
	if (whereBegin == whereLast)
		paging_physical_Bitmap[whereBegin] |= fillBits(begin % BITMAP_BITS, (end - 1) % BITMAP_BITS);
	else {
		paging_physical_Bitmap[whereBegin] |= fillBits(begin % BITMAP_BITS, BITMAP_BITS - 1);
		paging_physical_Bitmap[whereLast] |= fillBits(0, (end - 1) % BITMAP_BITS);
		for (int i = whereBegin + 1; i < whereLast; i++)
			paging_physical_Bitmap[i] = (~0ull);
	}
}

void paging_physical_BitmapWriteZero(int begin, int end) {
	int whereBegin = begin / BITMAP_BITS, whereLast = (end - 1) / BITMAP_BITS;
	if (whereBegin == whereLast)
		paging_physical_Bitmap[whereBegin] &= ~fillBits(begin % BITMAP_BITS, (end - 1) % BITMAP_BITS);
	else {
		paging_physical_Bitmap[whereBegin] &= ~fillBits(begin % BITMAP_BITS, BITMAP_BITS - 1);
		paging_physical_Bitmap[whereLast] &= ~fillBits(0, (end - 1) % BITMAP_BITS);
		for (int i = whereBegin + 1; i < whereLast; i++)
			paging_physical_Bitmap[i] = 0;
	}
}

static int phy_i = 1024 / 4 / BITMAP_BITS;

uint64_t paging_physical_AllocateOneFrame() {
	// skip the first 1M
	/*for (; phy_i < paging_EndPhysicalPage / BITMAP_BITS; phy_i++)
		if (paging_physical_Bitmap[phy_i] != ~0ull)
			for (int j = 0; j < BITMAP_BITS; j++)
				if ((paging_physical_Bitmap[phy_i] & (1ull << j)) == 0) {
					paging_physical_Bitmap[phy_i] |= (1ull << j);
					uint64_t addr = (((uint64_t)phy_i) * BITMAP_BITS + j) * SYSTEM_PAGE_SIZE;
					memset((void *)addr, 0, SYSTEM_PAGE_SIZE);
					return addr;
				}*/
	uint64_t addr;
	paging_physical_AllocateFrames(1, &addr);
	return addr;
}

// skip the first 2M
static int phy_i_2M = 2048 / 4 / BITMAP_BITS;

uint64_t paging_physical_AllocateOneFrame2M() {
	// skip the first 2M
	/*for (; phy_i_2M < paging_EndPhysicalPage / BITMAP_BITS; phy_i_2M += 8) {
		for (int j = 0; j < 8; j++)
			if (paging_physical_Bitmap[phy_i_2M + j] != 0)
				goto for_end;

		// now here we have a whole chunk at [i, i+8)
		for (int j = 0; j < 8; j++)
			paging_physical_Bitmap[phy_i_2M + j] = ~0ull;
		uint64_t addr = (((uint64_t)phy_i_2M) * BITMAP_BITS) * SYSTEM_PAGE_SIZE;
		memset((void *)addr, 0, SYSTEM_PAGE_2M_SIZE);
		return addr;
for_end:;
	}*/
	uint64_t addr;
	paging_physical_AllocateFrames2M(1, &addr);
	return addr;
}

int paging_physical_AllocateFrames(int pageCount, uint64_t frames[]) {
	// this code is in sync with paging_map.c, paging_map_PageAllocated, so you have to modify both
	int page = 0;
	for (; page < pageCount; page++) {
		frames[page] = 0;
		for (; phy_i < paging_EndPhysicalPage / BITMAP_BITS; phy_i++)
			if (paging_physical_Bitmap[phy_i] != ~0ull)
				for (int j = 0; j < BITMAP_BITS; j++)
					if ((paging_physical_Bitmap[phy_i] & (1ull << j)) == 0) {
						paging_physical_Bitmap[phy_i] |= (1ull << j);
						uint64_t addr = (((uint64_t)phy_i) * BITMAP_BITS + j) * SYSTEM_PAGE_SIZE;
						memset((void *)addr, 0, SYSTEM_PAGE_SIZE);
						frames[page] = addr;
						goto for_end; // break the phy_i loop
					}
		if (frames[page] == 0)
			break;
for_end:;
	}

	return page;
}

int paging_physical_AllocateFrames2M(int pageCount, uint64_t frames[]) {
	int page = 0;
	for (; page < pageCount; page++) {
		frames[page] = 0;
		for (; phy_i_2M < paging_EndPhysicalPage / BITMAP_BITS; phy_i_2M += 8) {
			for (int j = 0; j < 8; j++)
				if (paging_physical_Bitmap[phy_i_2M + j] != 0)
					goto for_end; // continue the outer for loop

			// now here we have a whole chunk at [i, i+8)
			for (int j = 0; j < 8; j++)
				paging_physical_Bitmap[phy_i_2M + j] = ~0ull;
			uint64_t addr = (((uint64_t)phy_i_2M) * BITMAP_BITS) * SYSTEM_PAGE_SIZE;
			memset((void *)addr, 0, SYSTEM_PAGE_2M_SIZE);

			frames[page] = addr;
			break;
for_end:;
		}
		if (frames[page] == 0)
			break;
	}
	return page;
}

void paging_physical_FreeFrame(uint64_t frame, int pageCount) {
	if (frame % SYSTEM_PAGE_SIZE != 0) {
		io_Printf("paging_physical_FreeFrame: frame %08llx is not aligned\n", frame);
		return;
	}
	paging_physical_BitmapWriteZero(frame / SYSTEM_PAGE_SIZE, frame / SYSTEM_PAGE_SIZE + pageCount);
}
