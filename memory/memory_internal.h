#pragma once

#include "memory.h"
#include "../efimain.h"


extern EFI_MEMORY_DESCRIPTOR *efiMemoryMap;
extern UINTN                  efiMemoryMapSize;
extern UINTN                  efiMemoryMapKey;
extern UINTN                  efiDescriptorSize;
extern UINT32                 efiDescriptorVertion;
