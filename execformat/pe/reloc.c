
#include "reloc.h"
#include "../../runtime/panic_assert.h"
#include <string.h>


static inline int __local_strncmp(const char *s1, const char *s2, size_t n) {
	while (n && *s1 && (*s1 == *s2)) {
		++s1;
		++s2;
		--n;
	}
	if (n == 0) {
		return 0;
	} else {
		return (*(unsigned char *)s1 - *(unsigned char *)s2);
	}
}

static const char *caseFlags(uint16_t type) {
#define CASE(expr)                       \
	case EXECFORMAT_PE_BASERELOC_##expr: \
		return #expr;

	switch (type) {
		CASE(ABSOLUTE)
		CASE(HIGH)
		CASE(LOW)
		CASE(HIGHLOW)
		CASE(HIGHADJ)
		CASE(DIR64)
		default:
			return "(Unknown)";
	}

#undef CASE
}

void execformat_pe_BaseRelocate(execformat_pe_PortableExecutable *pe, void *relocBase, void *relocEnd, uint64_t currentBase, uint64_t targetBase) {
	if (pe->begin == 0)
		return;

	if (currentBase == 0) {
		if (pe->isPE32P)
			currentBase = ((execformat_pe_OptionalHeader_PE32P *)pe->optional)->win.imageBase;
		else
			currentBase = ((execformat_pe_OptionalHeader_PE32 *)pe->optional)->win.imageBase;
	}

	void *reloc = 0, *reloc_end;
	if (relocBase != 0 && relocEnd != 0) {
		reloc     = relocBase;
		reloc_end = relocEnd;
	} else {
		// find the .reloc section
		for (int i = 0; i < pe->numSections; i++)
			if (__local_strncmp(pe->sections[i].name, ".reloc", 6) == 0) {
				reloc     = pe->begin + pe->sections[i].virtualAddr;
				reloc_end = reloc + pe->sections[i].virtualSize;
				break;
			}
		if (reloc == 0)
			return; // no .reloc section
	}

	uint64_t diff = targetBase - currentBase;

	while (reloc < reloc_end) {
		execformat_pe_BaseRelocBlock *block   = reloc;
		int                           entries = (block->blockSize - 8) / 2;
		assert((block->blockSize - 8) % 2 == 0);

		printf("BaseReloc Block RVA=0x%08x, entries=%d\n", block->pageOffset, entries);

		for (int i = 0; i < entries; i++) {
			void *target = pe->begin + block->pageOffset + (block->entries[i] & EXECFORMAT_PE_BASERELOC_OFFSET_MASK);
			//uint64_t cur = 0, new = 0;
			switch (block->entries[i] & EXECFORMAT_PE_BASERELOC_FLAG_MASK) {
				case EXECFORMAT_PE_BASERELOC_HIGH:
					//cur = *((uint16_t *)target);
					*((uint16_t *)target) += (uint16_t)(diff >> 16);
					//new = *((uint16_t *)target);
					break;
				case EXECFORMAT_PE_BASERELOC_LOW:
					//cur = *((uint16_t *)target);
					*((uint16_t *)target) += (uint16_t)(diff);
					//new = *((uint16_t *)target);
					break;
				case EXECFORMAT_PE_BASERELOC_HIGHLOW:
					//cur = *((uint32_t *)target);
					*((uint32_t *)target) += (uint32_t)(diff);
					//new = *((uint32_t *)target);
					break;
				case EXECFORMAT_PE_BASERELOC_DIR64:
					//cur = *((uint64_t *)target);
					*((uint64_t *)target) += (diff);
					//new = *((uint64_t *)target);
					break;
				case EXECFORMAT_PE_BASERELOC_HIGHADJ:
					i++;
					break;
			}
			//printf("    Reloc Off0x%08llx CUR=0x%08llx(Off0x%08llx), NEW=0x%llx, Type=%s\n", target - pe->begin, cur, cur - currentBase, new, caseFlags(block->entries[i] & EXECFORMAT_PE_BASERELOC_FLAG_MASK));
		}

		reloc += block->blockSize;
	}
}
