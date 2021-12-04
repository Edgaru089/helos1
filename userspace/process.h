#pragma once

#include "../main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


// Process is a userspace entity with distinct memory mapping, file descriptors, environment variables, and more.
// It latches itself on a smp_Thread.
typedef int userspace_ID;

// Init is called from kernel space to initialize the userspace.
//
// It calls the "init process" with a PID of 1.
void userspace_Init(const char *init, const char *environ);


#ifdef __cplusplus
}
#endif
