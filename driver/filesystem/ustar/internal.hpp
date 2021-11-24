#pragma once

#include "../../../main.h"
#include "../filesystem.hpp"
#include <stdint.h>

namespace helos {
namespace filesystem {


constexpr uint8_t
	__USTAR_Type_NormalFile      = '0', // '0' or '\0'(NUL)
	__USTAR_Type_HardLink        = '1',
	__USTAR_Type_SymbolicLink    = '2',
	__USTAR_Type_CharacterDevice = '3',
	__USTAR_Type_BlockDevice     = '4',
	__USTAR_Type_Directory       = '5',
	__USTAR_Type_FIFO            = '6'; // Named pipe (FIFO)

constexpr char __USTAR_Magic[] = "ustar", __USTAR_Version[] = "00";

// USTAR file structure
struct __USTAR_Metadata {
	char     filename[100];  // Filename, terminated by '\0'
	char     filemode[8];    // File mode, base-8 ASCII
	char     owner[8];       // Owner numeric ID, base-8 ASCII
	char     group[8];       // Group numeric ID, base-8 ASCII
	char     size[12];       // File size, in base-8 ASCII (e.g., 1025 = "000000002001")
	char     lastmodify[12]; // Last modify UNIX time, also base-8 ASCII
	char     checksum[8];    // Checksum for header
	uint8_t  type;           // File type, one of __USTAR_Type_*
	char     magic[6];       // Magic string "ustar\0" (lower case)
	char     version[2];     // USTAR Version, "00"
	char     linkname[100];  // Linked file name (hard link or symlink)
	char     ownername[32];  // Owner username (ASCII text)
	char     groupname[32];  // Group name (text)
	uint64_t deviceMajor;    // Device major number, base-8 ASCII
	uint64_t deviceMinor;    // Device minor number, base-8 ASCII
	char     prefix[155];    // Filename prefix, if any
} PACKED;

static_assert(sizeof(__USTAR_Metadata) == 500, "__USTAR_Metadata not packed");

// USTAR file information in memory (POD)
struct __USTAR_File {
	uintptr_t          offset;     // Offset in blocks of the real file data
	char              *filename;   // Filename, begin with '/', allocated
	uint64_t           size;       // Size in bytes of the file
	uint64_t           lastmodify; // Last modify UNIX time
	Filesystem::mode_t mode;       // File mode (rwxrwxrwx and file type)
	uint16_t           owner;      // Owner ID
	uint16_t           group;      // Group ID
};


} // namespace filesystem
} // namespace helos
