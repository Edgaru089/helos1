#pragma once

#include "../main.h"

// this function is here, well, mostly just for fun.
//
// userspace in the far future should need this
SYSV_ABI long Syscall(int id, long a, long b, long c, long d, long e, long f);
