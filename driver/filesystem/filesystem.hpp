#pragma once

#include "../../cppruntime/runtime.hpp"
#include "../block/blockdevice.hpp"

namespace helos {

namespace filesystem {


// Filesystems are created per-mount, or on filesystem register only for the Allocate functions.
//
// Paths always begin with "/".
// Functions return negative numbers on error.
//
// This class does not facilitate utilties to format, check, or resize filesystems.
class Filesystem {
public:
	typedef uint16_t uid_t, gid_t; // User and group ID types
	typedef uint16_t mode_t;       // UNIX permission mode type

	enum OpenFlag {
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
	};

public:
	static constexpr char Seperator = '/';

	// Error enums are positive, so use something like "return -EPERM;"
	enum Error {
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

	};
	// Configuration passed to New() from the kernel.
	struct Config {
		bool  setgid, setuid; // If true, GID/UID of each file is overwritten with gid/uid.
		gid_t gid;
		uid_t uid;

		bool   setfmask, setdmask; // If true, file/directory permissions are masked with the given values.
		mode_t fmask, dmask;       // (Corresponding bits cleared)

		bool readonly; // If true, the filesystem should be opened read-only, and no writes should be done.
	};

	// Information on an open file.
	struct OpenFile {
		// File open flags.
		OpenFlag flags;

		// Filled in by open(), to indicate the file is not seekable.
		bool         nonseekable : 1;
		unsigned int padding : 31;
		unsigned int padding2 : 32;

		int errno; // errno of the last operation, only need to be set for Read()/Write()

		// File handle. Filled in by Create(), Open(), Opendir().
		uint64_t handle;
	};

	enum RenameType {
		Rename_Exchange,  // Atomically swaps the 2 files if the target file exists.
		Rename_NoReplace, // Return with error if the target file exists.
	};

	// POSIX file types
	static constexpr mode_t S_IFMT   = 0170000; // Bit mask for the file type bit field
	static constexpr mode_t S_IFIFO  = 0010000; // FIFO
	static constexpr mode_t S_IFCHR  = 0020000; // Character device
	static constexpr mode_t S_IFDIR  = 0040000; // Directory
	static constexpr mode_t S_IFBLK  = 0060000; // Block device
	static constexpr mode_t S_IFREG  = 0100000; // Regular file
	static constexpr mode_t S_IFLNK  = 0120000; // Symbolic link
	static constexpr mode_t S_IFSOCK = 0140000; // Socket

	// File status
	struct Stat {
		mode_t   Mode;                         // File type and mode
		uint32_t NumLinks;                     // Number of hard links on the same file
		uid_t    Uid;                          // User ID of owner
		gid_t    Gid;                          // Group ID of owner
		uint64_t Size;                         // Total size of file in bytes
		uint64_t Blocks;                       // Number of 512Byte blocks allocated
		uint64_t TimeAccess, TimeModification; // UNIX time of last access/modification
	};

	// Allocate is called to create a new Filesystem instance from a given source.
	//
	// The source paramater is quite special.
	// Beginning with "/" means a local file source (device or image).
	// Beginning with a "//" means a network target (URL).
	// Otherwise, this string is non-canonical and its behavior is implementation-depedent.
	//
	// On error, NULL is returned.
	virtual Filesystem *Allocate(const char *source, Config *config) { return nullptr; }

	// AllocateBlock is called to create a new Filesystem instance from a Block Device.
	//
	// On error, NULL is returned.
	virtual Filesystem *AllocateBlock(BlockDevice *block, Config *config) { return nullptr; }

public:
	// Filesystem implementation capabilitiy bits
	enum Capability {
		Capability_AtomicTruncate = 1 << 0, // supports O_TRUNC open flag. If disabled, kernel calls Truncate() and Open() on O_TRUNC.
		Capability_Export         = 1 << 1, // supports lookup of "." and "..".
		Capability_NoOpen         = 1 << 2, // does not use Open()/Opendir() (returning ENOSYS), using stateless file I/O
	};

	virtual Capability Capabilities() { return Capability_NoOpen; }

public:
	virtual const char *GetFilesystemType() { return "(Default)"; }

public:
	virtual ~Filesystem() {}

	// Getattr is similar to stat(), returning file status.
	//
	// file is NULL if the file is not opened, but may too be NULL
	// if the file is opened.
	virtual int Getattr(const char *path, Stat *stat, OpenFile *file) { return -ENOSYS; }

	// Readlink reads the target of a symbolic link.
	//
	// If the linkname is too long to fit in the target, it should be truncated,
	// returning the required size of the new buffer.
	//
	// Returns 0 on success.
	virtual int Readlink(const char *link, char *buffer, uint64_t bufferSize) { return -ENOSYS; }

	// Create a new file node (empty file), without opening it.
	//
	// This is called for non-directory, non-symlink nodes.
	virtual int Mknod(const char *file, mode_t mode) { return -ENOSYS; }

	// Create a new directory.
	//
	// The mode argument might not have the S_IFDIR bits set.
	virtual int Mkdir(const char *path, mode_t mode) { return -ENOSYS; }

	// Remove a file or node.
	virtual int Unlink(const char *path) { return -ENOSYS; }

	// Remove an empty directory.
	virtual int Rmdir(const char *path) { return -ENOSYS; }

	// Create a symbolic link.
	virtual int Symlink(const char *target, const char *path) { return -ENOSYS; }

	// Rename (move) a file.
	//
	// The Type argument should be honored.
	virtual int Rename(const char *from, const char *to, RenameType type) { return -ENOSYS; }

	// Create a hard link to file.
	virtual int Link(const char *from, const char *to) { return -ENOSYS; }

	// Change the owner/group of a file.
	//
	// file is NULL if the file is not opened, but may too be NULL
	// if the file is opened.
	//
	// This function is never called if setuid/gid bits are set.
	virtual int Chown(const char *path, uid_t uid, gid_t gid, OpenFile *file) { return -ENOSYS; }

	// Change the permissions of a file.
	//
	// file is NULL if the file is not opened, but may too be NULL
	// if the file is opened.
	virtual int Chmod(const char *path, mode_t mode, OpenFile *file) { return -ENOSYS; }

	// Change the size of a file.
	//
	// file is NULL if the file is not opened, but may too be NULL
	// if the file is opened.
	virtual int Truncate(const char *path, uint64_t size, OpenFile *file) { return -ENOSYS; }

	// Opens a file, with flags in file->flags.
	//
	// - Creation flags (O_CREAT, O_EXCL, O_NOCTTY) are handled by the kernel and filtered out.
	// - Access modes (O_RDONLY, O_WRONLY,O_RDWR,O_EXEC) should be checked if the operation is permitted.
	//
	// Driver may store arbitrary value in file->handle and other flags to change the way the file is opened.
	virtual int Open(const char *path, OpenFile *file) { return -ENOSYS; }

	// Read data from an open file.
	//
	// Read should return exactly the number of bytes requested except
	// on EOF or error, when the rest of the data will be zeroed.
	virtual int Read(const char *path, char *buffer, uint64_t count, uint64_t offset, OpenFile *file) { return -ENOSYS; }

	// Write data to an open file.
	// Read should return exactly the number of bytes requested except on error.
	virtual int Write(const char *path, const char *buffer, uint64_t count, uint64_t offset, OpenFile *file) { return -ENOSYS; }

	// Syncs file data to disk.
	//
	// If syncOnlyUserData is true, sync only user data, not the metadata
	virtual int Fsync(const char *path, bool syncOnlyUserData, OpenFile *file) { return -ENOSYS; }

	// Closes an open file.
	//
	// For each Open() call there is exactly one Close() call with the same file->flags/handle.
	// It is possible that one same file has been opened several times.
	virtual int Close(const char *path, OpenFile *file) { return -ENOSYS; }

	// Open a directory for reading.
	//
	// This function should check for permissions in file->flags.
	virtual int Opendir(const char *path, OpenFile *file) { return -ENOSYS; }

	enum Readdir_CallbackFlags {
		Readdir_CallbackFlags_StatValid = 1 << 0, // All stat members are valid
	};
	// typedef of the callback Readdir() uses to emit directory entries
	typedef int (*Readdir_Callback)(void *user, const char *name, const Stat *stat, Readdir_CallbackFlags flags);

	// Read an open directory.
	//
	// The implementation calls Callback() on all directory entries.
	virtual int Readdir(const char *path, void *user, Readdir_Callback callback, OpenFile *file) { return -ENOSYS; }

	// Syncs the contents of a directory.
	//
	// If syncOnlyUserData is true, sync only user data, not the metadata
	virtual int Fsyncdir(const char *path, bool syncOnlyUserData, OpenFile *file) { return -ENOSYS; }

	// Close an open directory.
	virtual int Closedir(const char *path, OpenFile *file) { return -ENOSYS; }

	// Ioctl (arbitrary action performed on the opened file)
	virtual uintptr_t Ioctl(const char *path, uintptr_t cmd, void *arg, OpenFile *file) { return -ENOSYS; }
};


} // namespace filesystem
} // namespace helos
