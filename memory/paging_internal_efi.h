#pragma once

#include "../efimain.h"


// defined in paging_init.c
extern EFI_MEMORY_DESCRIPTOR *efiMemoryMap;
extern UINTN                  efiMemoryMapSize;
extern UINTN                  efiMemoryMapKey;
extern UINTN                  efiDescriptorSize;
extern UINT32                 efiDescriptorVertion;

static inline const char *
	memoryTypeName(EFI_MEMORY_TYPE type) {
#define CASE(c) \
	case c:     \
		return #c;
	switch (type) {
		CASE(EfiReservedMemoryType)
		CASE(EfiLoaderCode)
		CASE(EfiLoaderData)
		CASE(EfiBootServicesCode)
		CASE(EfiBootServicesData)
		CASE(EfiRuntimeServicesCode)
		CASE(EfiRuntimeServicesData)
		CASE(EfiConventionalMemory)
		CASE(EfiUnusableMemory)
		CASE(EfiACPIReclaimMemory)
		CASE(EfiACPIMemoryNVS)
		CASE(EfiMemoryMappedIO)
		CASE(EfiMemoryMappedIOPortSpace)
		CASE(EfiPalCode)
		case EfiMaxMemoryType:
			return "EfiPersistentMemory";
	}
	return "(unknown)";
#undef CASE
}

#ifndef NEXT_MEMORY_DESCRITOR
#define NEXT_MEMORY_DESCRITOR(desc, size) ((EFI_MEMORY_DESCRIPTOR *)((char *)desc + size)))
#endif
