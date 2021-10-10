
#include "format.h"
#include "struct.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	uint32_t pageOffset;
	uint32_t blockSize; // the size in bytes of the Relocation Block, including the 2 header fields and the entries that follow
	uint16_t entries[1];
} PACKED execformat_pe_BaseRelocBlock;

#define EXECFORMAT_PE_BASERELOC_ABSOLUTE    (0x0000u) // The base relocation is skipped. This type can be used to pad a block.
#define EXECFORMAT_PE_BASERELOC_HIGH        (0x1000u) // The base relocation adds the high 16 bits of the difference to the 16-bit field at offset.
#define EXECFORMAT_PE_BASERELOC_LOW         (0x2000u) // The base relocation adds the low 16 bits of the difference to the 16-bit field at offset.
#define EXECFORMAT_PE_BASERELOC_HIGHLOW     (0x3000u) // The base relocation applies all 32 bits of the difference to the 32-bit field at offset.
#define EXECFORMAT_PE_BASERELOC_HIGHADJ     (0x4000u) // Weird, ignored (together with the next entry).
#define EXECFORMAT_PE_BASERELOC_DIR64       (0xa000u) // The base relocation applies the difference to the 64-bit field at offset.
#define EXECFORMAT_PE_BASERELOC_FLAG_MASK   (0xf000u)
#define EXECFORMAT_PE_BASERELOC_OFFSET_MASK (0x0fffu)

void execformat_pe_BaseRelocate(execformat_pe_PortableExecutable *pe, void *relocBase, void *relocEnd, uint64_t currentBase, uint64_t targetBase);


#ifdef __cplusplus
} // extern "C"
#endif
