#pragma once

#include "process.h"
#include "../smp/kthread.h"
#include "../smp/internal.h"

#include "../driver/filesystem/userspace.h"
#include "../util/tree.h"


typedef struct {
	__smp_Thread *thread; // Host thread. Must be alive.
	userspace_ID  pid;    // Process ID.

	tree_Tree *fd;      // File descriptors. maps uintptr_t to __filesystem_File*.
	char      *environ; // A list of "NAME=VAL\n".

	uint64_t paged; // PHYSICAL address to the root of the new paging table.

} __userspace_Process;
