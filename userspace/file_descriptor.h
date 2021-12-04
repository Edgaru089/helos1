#pragma once

#include "../main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	userspace_FileBackend_SeekWhence_Set,     // Relative to the beginning of the file.
	userspace_FileBackend_SeekWhence_Current, // Relative to the current read position.
	userspace_FileBackend_SeekWhence_End,     // Relative to the past-the-end position.
} userspace_FileBackend_SeekWhence;

// FileBackend describes a backend of a file descriptor.
// It is allocated per-file, every process.
typedef struct {
	void *user;                    // User data.
	uintptr_t (*Size)(void *user); // Size returns the size in bytes of the file. Is NULL if the file have no size.
	uintptr_t (*Tell)(void *user); // Tell returns the current position of the read. Is NULL if the file cannot be seeked.

	uintptr_t (*Seek)(void *user, uintptr_t offset, userspace_FileBackend_SeekWhence whence); // Seek.
} userspace_FileBackend;


#ifdef __cplusplus
}
#endif
