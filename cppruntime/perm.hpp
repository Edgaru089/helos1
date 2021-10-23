#pragma once

#include <cstdint>


namespace helos {

// Permission on block devices and files, in all-right Unix fashion.
// This must be in-line with MAP_PROT_EXEC/WRITE/READ and other Unix perms.
typedef uint16_t     Permission;
constexpr Permission PermExecute = 1 << 0;
constexpr Permission PermWrite   = 1 << 1;
constexpr Permission PermRead    = 1 << 2;


} // namespace helos
