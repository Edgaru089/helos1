#pragma once

#include "../../main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


// This file contains operations meant only for userspace use.

// Forward declaration. A pointer to the structure is kept.
struct __filesystem_File;

// Filesystem ERRNOs
typedef enum {
	EPERM        = 1,  // Operation not permitted
	ENOENT       = 2,  // No such file or directory
	EAGAIN       = 11, // Resource temporarily unavailable
	EACCES       = 13, // Permission denied
	ENOTBLK      = 15, // Blockdevice required
	EBUSY        = 16, // Device or resource busy
	EEXIST       = 17, // File exists
	EXDEV        = 18, // No such device
	ENODEV       = 19, // No such device
	ENOTDIR      = 20, // Not a directory
	EISDIR       = 21, // Is a directory
	EINVAL       = 22, // Invalid argument
	ETXTBSY      = 26, // Text file busy
	EFBIG        = 27, // File too large
	ENOSPC       = 28, // No space left on device
	ESPIPE       = 29, // Illegal seek
	EROFS        = 30, // Read-only file system
	EMLINK       = 31, // Too many links
	EPIPE        = 32, // Broken pipe
	ENAMETOOLONG = 36, // File name too long
	ENOLCK       = 37, // No locks available
	ENOSYS       = 38, // Function not implemented
	ENOTEMPTY    = 39, // Directory not empty
	ELOOP        = 40, // Too many levels of symbolic links
	EWOULDBLOCK  = 11, // Resource temporarily unavailable (call would block)
} filesystem_Errno;

// Flags for Open(), as in filesystem.hpp
typedef enum {
	O_APPEND    = 1 << 0,  // File opened in append mode
	O_ASYNC     = 1 << 1,  // Enable signal-driver I/O
	O_CREAT     = 1 << 2,  // Create the file if it does not exist
	O_DIRECTORY = 1 << 3,  // Fail if not a directory
	O_DSYNC     = 1 << 4,  // Operation data synced on return
	O_EXCL      = 1 << 5,  // Fail if the file exists in conjunction with O_CREAT
	O_LARGEFILE = 1 << 6,  // Allow use of off64_t
	O_NOCTTY    = 1 << 7,  // Does not switch process's controlling terminal
	O_NOFOLLOW  = 1 << 8,  // Does not follow symbolic link on the trailing component (basename)
	O_NONBLOCK  = 1 << 9,  // Open file in nonblocking mode
	O_SYNC      = 1 << 10, // Writes synced by file on return
	O_TRUNC     = 1 << 11, // Thuncate existing file to size 0 if it exists

	O_RDONLY = 1 << 12,             // Read-only
	O_WRONLY = 1 << 13,             // Write-only
	O_RDWR   = O_RDONLY | O_WRONLY, // Read and write
	O_EXEC   = 1 << 14,             // Execute
} filesystem_OpenFlag;

// Open opens a file. Returns negative ERRNO on errors. (*file) is filled in.
//
// If the file is a directory, its contents are buffered on first read.
intptr_t filesystem_Open(struct __filesystem_File **file, const char *path, filesystem_OpenFlag flags);

// Destroy disposes a open file structure, flushing the data.
intptr_t filesystem_Destroy(struct __filesystem_File *file);

// Adds (or Decrements) the internal reference count of the file.
// The new reference count is returned. It is initially 0.
//
// It does not dispose the structure automatically.
int filesystem_ReferenceChange(struct __filesystem_File *file, int count);

// Where to seek from for Seek().
typedef enum {
	SEEK_SET = 0,
	SEEK_CUR = 1,
	SEEK_END = 2,
} filesystem_SeekWhence;

// Seek performs a seek on the file. Returns negative ERRNO on errors, or the current read offset in bytes.
int64_t filesystem_Seek(struct __filesystem_File *file, uint64_t offset, filesystem_SeekWhence whence);

// Read reads a file. Returns the number of bytes read, or -ERRNO on error.
int64_t filesystem_Read(struct __filesystem_File *file, void *buffer, uintptr_t size);

// Write writes a file. Returns the number of bytes written, or -ERRNO on error.
int64_t filesystem_Write(struct __filesystem_File *file, void *buffer, uintptr_t size);

// Ioctl.
int64_t filesystem_Ioctl(struct __filesystem_File *file, uintptr_t opcode, void *param);

typedef uint16_t filesystem_uid_t, filesystem_gid_t; // User and group ID types
typedef uint16_t filesystem_mode_t;                  // UNIX permission mode type

typedef struct {
	uint64_t          Size;                         // Total size of file in bytes
	uint64_t          TimeAccess, TimeModification; // UNIX time of last access/modification
	uint64_t          Length;                       // Size in bytes of this entry
	filesystem_mode_t Mode;                         // File type and mode
	filesystem_uid_t  Uid;                          // User ID of owner
	filesystem_gid_t  Gid;                          // Group ID of owner
	char              Name[1];                      // Placeholder for NULL-terminated filename
} PACKED filesystem_DirectoryEntry;

// Reads (part of) a open directory. Returns the number of entries read, or -ERRNO on error.
// See Linux syscall manpage getdents(2).
int64_t filesystem_ReadDir(struct __filesystem_File *file, void *buffer, uintptr_t bufferSize);


#ifdef __cplusplus
}
#endif
