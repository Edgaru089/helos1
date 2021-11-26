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
	char     linkname[100];  // Linked file name (hard link or symlink)
	char     magic[6];       // Magic string "ustar\0" (lower case)
	char     version[2];     // USTAR Version, "00"
	char     ownername[32];  // Owner username (ASCII text)
	char     groupname[32];  // Group name (text)
	uint64_t deviceMajor;    // Device major number, base-8 ASCII
	uint64_t deviceMinor;    // Device minor number, base-8 ASCII
	char     prefix[155];    // Filename prefix, if any
} PACKED;

static_assert(sizeof(__USTAR_Metadata) == 500, "__USTAR_Metadata not packed");


} // namespace filesystem
} // namespace helos


#if 0
namespace {
template<int linesplit = 16>
inline void hexdump(void *buf, int size) {
	char str[linesplit + 1];
	for (int i = 0; i < size; i += linesplit) {
		io_Printf("%04x:", i);
		for (int j = 0; j < linesplit; j += 2)
			io_Printf(" %02x%02x", (uintptr_t)buf[i + j], (uintptr_t)buf[i + j + 1]);

		for (int j = 0; j < linesplit; j++) {
			char c = buf[i + j];
			if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
				str[j] = c;
			else
				str[j] = '.';
		}
		str[16] = 0;
		io_WriteConsoleASCII("  ");
		io_WriteConsoleASCII(str);
		io_WriteConsoleASCII("\n");
	}
}
} // namespace
#endif
