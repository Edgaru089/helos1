#pragma once

#include "../main.h"
#include <stdint.h>


// memory_AddBreak is a sbrk() clone, increasing the kernel heap size mapped at KERNEL_HEAP_VIRTUAL.
// TODO It does not handle negative increments well (yet), as it does not unmap freed heap pages.
//      I don't know if I should actually implement this, given it's a rather simple fix.
void *memory_AddBreak(intptr_t increment);
