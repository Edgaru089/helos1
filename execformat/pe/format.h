#pragma once

#include "../../main.h"
#include "../../runtime/panic_assert.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


// PE Header Magic, comes after the MS-DOS stub and right before PE Header
#define EXECFORMAT_PE_HEADER_MAGIC "PE\0" // comes with a \0 itself

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format
typedef struct {
	uint16_t machineType;
	uint16_t numSections;
	uint32_t unixTimestamp;
	uint32_t offsetSymbolTable;
	uint32_t numSymbols;
	uint16_t sizeofOptionalHeader;
	uint16_t flags; // "Characteristics"
} PACKED execformat_pe_Header;

// Machine types
#define EXECFORMAT_PE_MACHINE_UNKNOWN 0x0u    // The content of this field is assumed to be applicable to any machine type
#define EXECFORMAT_PE_MACHINE_AMD64   0x8664u // x64
#define EXECFORMAT_PE_MACHINE_I386    0x14cu  // Intel 386 or later processors and compatible processors
#define EXECFORMAT_PE_MACHINE_EBC     0xebcu  // EFI byte code

// Characteristics
#define EXECFORMAT_PE_FLAG_RELOCS_STRIPPED         0x0001u // base relocs removed and must be loaded at its preferred base address.
#define EXECFORMAT_PE_FLAG_EXECUTABLE_IMAGE        0x0002u // the image file is valid and can be run.
#define EXECFORMAT_PE_FLAG_LINE_NUMS_STRIPPED      0x0004u // COFF line numbers removed. deprecated and should be zero.
#define EXECFORMAT_PE_FLAG_LOCAL_SYMS_STRIPPED     0x0008u // COFF symbol table entries for local symbols removed. deprecated and should be zero.
#define EXECFORMAT_PE_FLAG_AGGRESSIVE_WS_TRIM      0x0010u // Aggressively trim working set. deprecated and later and must be zero.
#define EXECFORMAT_PE_FLAG_LARGE_ADDRESS_AWARE     0x0020u // Application can handle >2GB addresses.
#define EXECFORMAT_PE_FLAG_RESERVED_0x0040         0x0040u
#define EXECFORMAT_PE_FLAG_BYTES_REVERSED_LO       0x0080u // Little endian. This flag is deprecated and should be zero.
#define EXECFORMAT_PE_FLAG_32BIT_MACHINE           0x0100u // Machine is based on a 32-bit-word architecture.
#define EXECFORMAT_PE_FLAG_DEBUG_STRIPPED          0x0200u // Debugging information is removed from the image file.
#define EXECFORMAT_PE_FLAG_REMOVABLE_RUN_FROM_SWAP 0x0400u // If the image is on removable media, fully load it and copy it to the swap file.
#define EXECFORMAT_PE_FLAG_NET_RUN_FROM_SWAP       0x0800u // If the image is on network media, fully load it and copy it to the swap file.
#define EXECFORMAT_PE_FLAG_SYSTEM                  0x1000u // The image file is a system file, not a user program.
#define EXECFORMAT_PE_FLAG_DLL                     0x2000u // The image file is a dynamic-link library (DLL).
#define EXECFORMAT_PE_FLAG_UP_SYSTEM_ONLY          0x4000u // The file should be run only on a uniprocessor machine.
#define EXECFORMAT_PE_FLAG_BYTES_REVERSED_HI       0x8000u // Big endian. This flag is deprecated and should be zero.

// Optional Header Magic
#define EXECFORMAT_PE_OPTIONAL_HEADER_MAGIC_PE32  0x10bu // 2-byte magic for PE32 Optional Header, right following the PE Header.
#define EXECFORMAT_PE_OPTIONAL_HEADER_MAGIC_PE32P 0x20bu // 2-byte magic for PE32+ Optional Header, right following the PE Header.

typedef struct {
	uint16_t magic; // either 0x10b for PE32, or 0x20b for PE32+
	uint8_t  majorLinkerVersion, minorLinkerVersion;
	uint32_t sizeofText;         // size of the .text section, or the sum of all text sections
	uint32_t sizeofData;         // size of initialized data sections
	uint32_t sizeofBss;          // size of uninitialized data
	uint32_t offsetofEntryPoint; // offset of the entry point (starting address) relative to the image base
	uint32_t offsetofText;       // offset of the start of .text section relative to the image base
} PACKED execformat_pe_OptionalHeader_StandardFields_PE32P;

typedef struct {
	uint16_t magic; // either 0x10b for PE32, or 0x20b for PE32+
	uint8_t  majorLinkerVersion, minorLinkerVersion;
	uint32_t sizeofText;         // size of the .text section, or the sum of all text sections
	uint32_t sizeofData;         // size of initialized data sections
	uint32_t sizeofBss;          // size of uninitialized data
	uint32_t offsetofEntryPoint; // offset of the entry point (starting address) relative to the image base
	uint32_t offsetofText;       // offset of the start of .text section relative to the image base
	uint32_t offsetofData;       // only present in PE32 images
} PACKED execformat_pe_OptionalHeader_StandardFields_PE32;

// PE32+ struct with a 64-bit ImageBase
typedef struct {
	uint64_t imageBase;                      // The preferred address of the first byte of image when loaded into memory; must be aligned to 64K.
	uint32_t sectionAlignment;               // The alignment (in bytes) of sections when they are loaded into memory.
	uint32_t fileAlignment;                  // The alignment factor (in bytes) that is used to align the raw data of sections in the image file.
	uint16_t majorOSVersion, minorOSVersion; // The version number of the required operating system.
	uint16_t majorImageVersion, minorImageVersion;
	uint16_t majorSubsystemVersion, minorSubsystemVersion;
	uint32_t win32VersionValue; // Reserved, must be 0
	uint32_t sizeofImage;       // Size in bytes of the image, including all headers as the image is loaded into memory.
	uint32_t sizeofHeaders;     // Combined size of the MS-DOS stub, PE header, and section headers rounded up to FileAlignment.
	uint32_t checksum;          // Checksum of an unknown(???) algorithm
	uint16_t subsystem;         // Subsystem
	uint16_t dllFlags;          // DLL Characteristics
	uint64_t sizeofStackReserve, sizeofStackCommit;
	uint64_t sizeofHeapReserve, sizeofHeapCommit;
	uint32_t loaderFlags;    // Reserved, must be 0
	uint32_t numRVAandSizes; // The number of data-directory entries in the remainder of the optional header. Each describes a location and size.
} PACKED execformat_pe_OptionalHeader_WindowsFields_PE32P;

// PE32 struct with a 32-bit ImageBase
typedef struct {
	uint32_t imageBase;                      // The preferred address of the first byte of image when loaded into memory; must be aligned to 64K.
	uint32_t sectionAlignment;               // The alignment (in bytes) of sections when they are loaded into memory.
	uint32_t fileAlignment;                  // The alignment factor (in bytes) that is used to align the raw data of sections in the image file.
	uint16_t majorOSVersion, minorOSVersion; // The version number of the required operating system.
	uint16_t majorImageVersion, minorImageVersion;
	uint16_t majorSubsystemVersion, minorSubsystemVersion;
	uint32_t win32VersionValue; // Reserved, must be 0
	uint32_t sizeofImage;       // Size in bytes of the image, including all headers as the image is loaded into memory.
	uint32_t sizeofHeaders;     // Combined size of the MS-DOS stub, PE header, and section headers rounded up to FileAlignment.
	uint32_t checksum;          // Checksum of an unknown(???) algorithm
	uint16_t subsystem;         // Subsystem
	uint16_t dllFlags;          // DLL Characteristics
	uint32_t sizeofStackReserve, sizeofStackCommit;
	uint32_t sizeofHeapReserve, sizeofHeapCommit;
	uint32_t loaderFlags;    // Reserved, must be 0
	uint32_t numRVAandSizes; // The number of data-directory entries in the remainder of the optional header. Each describes a location and size.
} PACKED execformat_pe_OptionalHeader_WindowsFields_PE32;

// Subsystems
#define EXECFORMAT_PE_SUBSYSTEM_UNKNOWN                  0
#define EXECFORMAT_PE_SUBSYSTEM_NATIVE                   1  // Device drivers and native Windows processes
#define EXECFORMAT_PE_SUBSYSTEM_WINDOWS_GUI              2  // Windows graphical user interface (GUI) subsystem
#define EXECFORMAT_PE_SUBSYSTEM_WINDOWS_CUI              3  // Windows character subsystem
#define EXECFORMAT_PE_SUBSYSTEM_OS2_CUI                  5  // OS/2 character subsystem
#define EXECFORMAT_PE_SUBSYSTEM_POSIX_CUI                7  // POSIX character subsystem
#define EXECFORMAT_PE_SUBSYSTEM_NATIVE_WINDOWS           8  // Native Win9x driver
#define EXECFORMAT_PE_SUBSYSTEM_WINDOWS_CE_GUI           9  // Windows CE
#define EXECFORMAT_PE_SUBSYSTEM_EFI_APPLICATION          10 // Extensible Firmware Interface (EFI) application
#define EXECFORMAT_PE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER  11 // EFI Boot Services driver
#define EXECFORMAT_PE_SUBSYSTEM_EFI_RUNTIME_DRIVER       12 // EFI Runtime driver
#define EXECFORMAT_PE_SUBSYSTEM_EFI_ROM                  13 // EFI ROM image
#define EXECFORMAT_PE_SUBSYSTEM_XBOX                     14 // XBOX
#define EXECFORMAT_PE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION 16 // Windows boot application

// DLL Characteristics
#define EXECFORMAT_PE_DLLFLAG_RESERVED_0x0001       0x0001u // Reserved, must be 0
#define EXECFORMAT_PE_DLLFLAG_RESERVED_0x0002       0x0002u // Reserved, must be 0
#define EXECFORMAT_PE_DLLFLAG_RESERVED_0x0004       0x0004u // Reserved, must be 0
#define EXECFORMAT_PE_DLLFLAG_RESERVED_0x0008       0x0008u // Reserved, must be 0
#define EXECFORMAT_PE_DLLFLAG_HIGH_ENTROPY_VA       0x0020u // Image can handle a high entropy 64-bit virtual address space.
#define EXECFORMAT_PE_DLLFLAG_DYNAMIC_BASE          0x0040u // DLL can be relocated at load time.
#define EXECFORMAT_PE_DLLFLAG_FORCE_INTEGRITY       0x0080u // Code Integrity checks are enforced.
#define EXECFORMAT_PE_DLLFLAG_NX_COMPAT             0x0100u // Image is NX compatible.
#define EXECFORMAT_PE_DLLFLAG_NO_ISOLATION          0x0200u // Isolation aware, but do not isolate the image.
#define EXECFORMAT_PE_DLLFLAG_NO_SEH                0x0400u // Does not use structured exception (SE) handling.
#define EXECFORMAT_PE_DLLFLAG_NO_BIND               0x0800u // Do not bind the image.
#define EXECFORMAT_PE_DLLFLAG_APPCONTAINER          0x1000u // Image must execute in an AppContainer.
#define EXECFORMAT_PE_DLLFLAG_WDM_DRIVER            0x2000u // A WDM driver.
#define EXECFORMAT_PE_DLLFLAG_GUARD_CF              0x4000u // Image supports Control Flow Guard.
#define EXECFORMAT_PE_DLLFLAG_TERMINAL_SERVER_AWARE 0x8000u // Terminal Server aware.

typedef struct {
	uint32_t Offset;
	uint32_t Size;
} PACKED execformat_pe_OptionalHeader_DataDirectory;

// Data Directory entry indexes
#define EXECFORMAT_PE_DATADIR_INDEX_EXPORT             0 // .edata
#define EXECFORMAT_PE_DATADIR_INDEX_IMPORT             1 // .idata
#define EXECFORMAT_PE_DATADIR_INDEX_RESOURCE           2 // .rsrc
#define EXECFORMAT_PE_DATADIR_INDEX_EXCEPTION          3 // .pdata
#define EXECFORMAT_PE_DATADIR_INDEX_CERTIFICATE        4
#define EXECFORMAT_PE_DATADIR_INDEX_BASE_RELOCATION    5 // .reloc
#define EXECFORMAT_PE_DATADIR_INDEX_DEBUG              6 // .debug
#define EXECFORMAT_PE_DATADIR_INDEX_ARCHITECTURE       7 // Reserved, must be 0
#define EXECFORMAT_PE_DATADIR_INDEX_GLOBAL_PTR         8 // The RVA of the value to be stored in the global pointer register. The size must be 0.
#define EXECFORMAT_PE_DATADIR_INDEX_TLS                9 // .tls
#define EXECFORMAT_PE_DATADIR_INDEX_LOAD_CONFIG        10
#define EXECFORMAT_PE_DATADIR_INDEX_BOUND_IMPORT       11
#define EXECFORMAT_PE_DATADIR_INDEX_IAT                12 // Import Address Table
#define EXECFORMAT_PE_DATADIR_INDEX_DELAY_IMPORT_DESC  13 // Delay Import Descriptor
#define EXECFORMAT_PE_DATADIR_INDEX_CLR_RUNTIME_HEADER 14 // Common Language Runtime header
#define EXECFORMAT_PE_DATADIR_INDEX_RESERVED_15        15 // Reserved, must be 0

typedef struct {
	execformat_pe_OptionalHeader_StandardFields_PE32P std;
	execformat_pe_OptionalHeader_WindowsFields_PE32P  win;
	execformat_pe_OptionalHeader_DataDirectory        data[1];
} PACKED execformat_pe_OptionalHeader_PE32P;

typedef struct {
	execformat_pe_OptionalHeader_StandardFields_PE32 std;
	execformat_pe_OptionalHeader_WindowsFields_PE32  win;
	execformat_pe_OptionalHeader_DataDirectory       data[1];
} PACKED execformat_pe_OptionalHeader_PE32;

static inline void execformat_pe_OptionalHeader_CheckPacking() {
	assert(sizeof(execformat_pe_OptionalHeader_StandardFields_PE32) == 28);
	assert(sizeof(execformat_pe_OptionalHeader_StandardFields_PE32P) == 24);
	assert(sizeof(execformat_pe_OptionalHeader_WindowsFields_PE32) == 68);
	assert(sizeof(execformat_pe_OptionalHeader_WindowsFields_PE32P) == 88);

	assert(offsetof(execformat_pe_OptionalHeader_PE32, std) == 0);
	assert(offsetof(execformat_pe_OptionalHeader_PE32, win) == 28);
	assert(offsetof(execformat_pe_OptionalHeader_PE32, data[0]) == 96);
	assert(offsetof(execformat_pe_OptionalHeader_PE32P, std) == 0);
	assert(offsetof(execformat_pe_OptionalHeader_PE32P, win) == 24);
	assert(offsetof(execformat_pe_OptionalHeader_PE32P, data[0]) == 112);
}


typedef struct {
	char     name[8];
	uint32_t virtualSize;
	uint32_t virtualAddr; // Offset(RVA) instead of a real Virtual Address
	uint32_t sizeofRawData;
	uint32_t pointerToRawData; // The file pointer to the first page of the section within the file.
	uint32_t pointerToRelocations;
	uint32_t pointerToLineNumbers;
	uint16_t numRelocations;
	uint16_t numLineNumbers; // deprecated, should be 0
	uint32_t flags;          // The flags that describe the characteristics of the section.
} PACKED execformat_pe_SectionHeader;

// Section Characteristics
#define EXECFORMAT_PE_SECTIONFLAG_NO_PAD                 0x00000008u // section should not be padded to the next boundary, obsolete.
#define EXECFORMAT_PE_SECTIONFLAG_CNT_CODE               0x00000020u // section contains executable code.
#define EXECFORMAT_PE_SECTIONFLAG_CNT_INITIALIZED_DATA   0x00000040u // section contains initialized data.
#define EXECFORMAT_PE_SECTIONFLAG_CNT_UNINITIALIZED_DATA 0x00000080u // section contains uninitialized data.
#define EXECFORMAT_PE_SECTIONFLAG_LNK_OTHER              0x00000100u // Reserved.
#define EXECFORMAT_PE_SECTIONFLAG_LNK_INFO               0x00000200u // section contains comments or other info. .drectve section has this type.
#define EXECFORMAT_PE_SECTIONFLAG_LNK_REMOVE             0x00000800u // section will not become part of the image. valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_LNK_COMDAT             0x00001000u // The section contains COMDAT data. valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_GPREL                  0x00008000u // section contains data referenced through the global pointer (GP).
#define EXECFORMAT_PE_SECTIONFLAG_MEM_PURGEABLE          0x00020000u // Reserved.
#define EXECFORMAT_PE_SECTIONFLAG_MEM_16BIT              0x00020000u // Reserved.
#define EXECFORMAT_PE_SECTIONFLAG_MEM_LOCKED             0x00040000u // Reserved.
#define EXECFORMAT_PE_SECTIONFLAG_MEM_PRELOAD            0x00080000u // Reserved.

// Section Characteristics - Alignment (only for object files)
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_1BYTES    0x00100000u // Align data on a 1-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_2BYTES    0x00200000u // Align data on a 2-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_4BYTES    0x00300000u // Align data on a 4-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_8BYTES    0x00400000u // Align data on a 8-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_16BYTES   0x00500000u // Align data on a 16-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_32BYTES   0x00600000u // Align data on a 32-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_64BYTES   0x00700000u // Align data on a 64-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_128BYTES  0x00800000u // Align data on a 128-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_256BYTES  0x00900000u // Align data on a 256-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_512BYTES  0x00a00000u // Align data on a 512-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_1024BYTES 0x00b00000u // Align data on a 1024-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_2048BYTES 0x00c00000u // Align data on a 2048-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_4096BYTES 0x00d00000u // Align data on a 4096-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_8192BYTES 0x00e00000u // Align data on a 8192-byte boundary. Valid only for object files.
#define EXECFORMAT_PE_SECTIONFLAG_ALIGN_MASK      0x00f00000u // Mask for section data alignment type.

// Section Characteristics - Memory
#define EXECFORMAT_PE_SECTIONFLAG_LNK_NRELOC_OVFL 0x01000000u // section contains extended relocations.
#define EXECFORMAT_PE_SECTIONFLAG_MEM_DISCARDABLE 0x02000000u // section can be discarded as needed.
#define EXECFORMAT_PE_SECTIONFLAG_MEM_NOT_CACHED  0x04000000u // section cannot be cached.
#define EXECFORMAT_PE_SECTIONFLAG_MEM_NOT_PAGED   0x08000000u // section cannot be paged out.
#define EXECFORMAT_PE_SECTIONFLAG_MEM_SHARED      0x10000000u // section can be shared in memory.
#define EXECFORMAT_PE_SECTIONFLAG_MEM_EXECUTE     0x20000000u // section can be executed as code.
#define EXECFORMAT_PE_SECTIONFLAG_MEM_READ        0x40000000u // section can be read.
#define EXECFORMAT_PE_SECTIONFLAG_MEM_WRITE       0x80000000u // section can be written to.


#ifdef __cplusplus
} // extern "C"
#endif
