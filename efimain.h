#pragma once

#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#include <efi.h>
}
#else
#include <efi.h>
#endif


extern EFI_HANDLE         efiImageHandle;
extern EFI_SYSTEM_TABLE  *efiSystemTable;
extern EFI_BOOT_SERVICES *efiBootServices;

extern SIMPLE_TEXT_OUTPUT_INTERFACE *efiStdout, *efiStderr;
extern SIMPLE_INPUT_INTERFACE       *efiStdin;
