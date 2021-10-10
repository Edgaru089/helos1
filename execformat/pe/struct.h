#pragma once

#include "../../main.h"
#include "format.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	void *                       begin;      // beginning of the image
	char *                       pemagic;    // PE magic "PE\0\0", after the MS-DOS stub
	execformat_pe_Header *       header;     // PE headers, 4 bytes right after the magic
	void *                       optional;   // optional headers, converted to OptionalHeader_PE32/PE32P on access
	execformat_pe_SectionHeader *sections;   // start of the section header tables
	uint32_t                     size;       // size of the file in bytes
	int                          numDataDir; // shorthand for ((execformat_pe_OptionalHeader_PE32P*)pe->optional)->win.numRVAandSizes
	int                          numSections;
	bool                         isPE32P;
} execformat_pe_PortableExecutable;

void execformat_pe_LoadMemory(execformat_pe_PortableExecutable *pe, void *image, uint32_t size);


#ifdef __cplusplus
} // extern "C"
#endif
