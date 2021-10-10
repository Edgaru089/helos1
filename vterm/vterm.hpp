#pragma once

#include "../main.h"

#include "../memory/memory.h"
#include "../memory/memory.hpp"
#include "../extlib/libvterm/vterm.h"
#include <cstring>


namespace helos {


class Terminal {
public:
	Terminal() {
	}
};


extern VTermAllocatorFunctions vterm_Allocator;


} // namespace helos

extern "C" {
FASTCALL_ABI void vterm_Init();
}
