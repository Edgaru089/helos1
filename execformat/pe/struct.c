
#include "struct.h"

#include "../../runtime/panic_assert.h"
#include <string.h>


void execformat_pe_LoadMemory(execformat_pe_PortableExecutable *pe, void *image, uint32_t size) {
	pe->begin = pe->pemagic = 0;
	if (size < 2 || ((char *)image)[0] != 'M' || ((char *)image)[1] != 'Z')
		return;

	// look for the "PE\0\0" magic on a 8-byte boundary
	for (int i = 8; i < size; i++) {
		if (memcmp(image + i, EXECFORMAT_PE_HEADER_MAGIC, 4) == 0) {
			pe->pemagic = image + i;
			break;
		}
	}
	if (pe->pemagic == 0)
		return; // not found

	pe->header      = (execformat_pe_Header *)(pe->pemagic + 4);
	pe->numSections = pe->header->numSections;
	pe->optional    = (void *)pe->header + sizeof(execformat_pe_Header);
	pe->isPE32P     = (*((uint16_t *)pe->optional) == EXECFORMAT_PE_OPTIONAL_HEADER_MAGIC_PE32P);

	if (pe->isPE32P) {
		pe->numDataDir = ((execformat_pe_OptionalHeader_PE32P *)pe->optional)->win.numRVAandSizes;
		pe->sections =
			pe->optional +
			sizeof(execformat_pe_OptionalHeader_StandardFields_PE32P) +
			sizeof(execformat_pe_OptionalHeader_WindowsFields_PE32P) +
			sizeof(execformat_pe_OptionalHeader_DataDirectory) * pe->numDataDir;

		assert((void *)pe->sections - pe->optional == pe->header->sizeofOptionalHeader && "PE32P OptionalHeader size mismatch");
	} else {
		pe->numDataDir = ((execformat_pe_OptionalHeader_PE32 *)pe->optional)->win.numRVAandSizes;
		pe->sections =
			pe->optional +
			sizeof(execformat_pe_OptionalHeader_StandardFields_PE32) +
			sizeof(execformat_pe_OptionalHeader_WindowsFields_PE32) +
			sizeof(execformat_pe_OptionalHeader_DataDirectory) * pe->numDataDir;

		assert((void *)pe->sections - pe->optional == pe->header->sizeofOptionalHeader && "PE32 OptionalHeader size mismatch");
	}

	pe->begin = image;
	pe->size  = size;
}
