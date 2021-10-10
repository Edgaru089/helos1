
#include "format.h"
#include "struct.h"
#include "../../memory/memory.h"

#include <string.h>

static const char *caseMachine(uint16_t type) {
#define CASE(expr)             \
	case EXECFORMAT_PE_##expr: \
		return #expr;

	switch (type) {
		CASE(MACHINE_UNKNOWN)
		CASE(MACHINE_AMD64)
		CASE(MACHINE_I386)
		CASE(MACHINE_EBC)
		default:
			return "(Unknown)";
	}

#undef CASE
}

static void caseFlags(uint32_t flags) {
#define CASE(expr)                           \
	if (flags & EXECFORMAT_PE_FLAG_##expr) { \
		printf("|" #expr);                   \
	}
	CASE(RELOCS_STRIPPED)
	CASE(EXECUTABLE_IMAGE)
	CASE(LINE_NUMS_STRIPPED)
	CASE(LOCAL_SYMS_STRIPPED)
	CASE(AGGRESSIVE_WS_TRIM)
	CASE(LARGE_ADDRESS_AWARE)
	CASE(RESERVED_0x0040)
	CASE(BYTES_REVERSED_LO)
	CASE(32BIT_MACHINE)
	CASE(DEBUG_STRIPPED)
	CASE(REMOVABLE_RUN_FROM_SWAP)
	CASE(NET_RUN_FROM_SWAP)
	CASE(SYSTEM)
	CASE(DLL)
	CASE(UP_SYSTEM_ONLY)
	CASE(BYTES_REVERSED_HI)
	printf("|\n");
#undef CASE
}
static void caseSectionFlags(uint32_t flags) {
#define CASE(expr)                                  \
	if (flags & EXECFORMAT_PE_SECTIONFLAG_##expr) { \
		printf("|" #expr);                          \
	}
	CASE(NO_PAD)
	CASE(CNT_CODE)
	CASE(CNT_INITIALIZED_DATA)
	CASE(CNT_UNINITIALIZED_DATA)
	CASE(LNK_INFO)
	CASE(LNK_REMOVE)
	CASE(LNK_COMDAT)
	CASE(GPREL)
	CASE(LNK_NRELOC_OVFL)
	CASE(MEM_DISCARDABLE)
	CASE(MEM_NOT_CACHED)
	CASE(MEM_NOT_PAGED)
	CASE(MEM_SHARED)
	CASE(MEM_EXECUTE)
	CASE(MEM_READ)
	CASE(MEM_WRITE)
	printf("|\n");
#undef CASE
}


void execformat_pe_ReadSystemHeader(execformat_pe_PortableExecutable *pe) {
	execformat_pe_OptionalHeader_CheckPacking();

	uint64_t a;
	asm volatile("leaq (%%rip), %0"
				 : "=r"(a));
	io_Printf("Stack position: %llx, RIP=%llx\n", &a, a);

	void *addr = (void *)paging_LoaderCodeAddress;

	execformat_pe_PortableExecutable rpe;
	if (pe == 0)
		pe = &rpe;
	execformat_pe_LoadMemory(pe, addr, paging_LoaderCodeSize);

	printf("%s Executable, Machine Type=0x%x (%s), Flags=", pe->isPE32P ? "PE32+" : "PE32", pe->header->machineType, caseMachine(pe->header->machineType));
	caseFlags(pe->header->flags);
	printf("\nImageBase=%d (0x%x)\n", ((execformat_pe_OptionalHeader_PE32P *)pe->optional)->win.imageBase, ((execformat_pe_OptionalHeader_PE32P *)pe->optional)->win.imageBase);

	printf("numRVAandSizes=%d (want %d)\n\n", pe->numDataDir, EXECFORMAT_PE_DATADIR_INDEX_CLR_RUNTIME_HEADER + 2);

	printf("numSections=%d\n", pe->numSections);
	char name[9];
	for (int i = 0; i < pe->numSections; i++) {
		name[8] = 0;
		strncpy(name, pe->sections[i].name, 8);

		printf("   %8s RVA0x%08x File0x%08x Len0x%08x ", name, pe->sections[i].virtualAddr, pe->sections[i].pointerToRawData, pe->sections[i].virtualSize);

		caseSectionFlags(pe->sections[i].flags);
	}
}
