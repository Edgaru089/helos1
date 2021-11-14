#pragma once

#include "channel.h"


// __smp_Channel_Waiter goes into Channel.send/recv
typedef struct {
	void *         buffer;
	uintptr_t      count; // object count, NOT bytes
	smp_Condition *cond;
} __smp_Channel_Waiter;
