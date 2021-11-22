#pragma once

#include "../../../cppruntime/runtime.hpp"
#include "../filesystem.hpp"
#include "../../../util/vector.h"
#include "../../../cppruntime/string.hpp"

namespace helos {
namespace filesystem {


// VirtualFilesystem is the global virtual filesystem.
class VirtualFilesystem: public Filesystem {
public:
	VirtualFilesystem();
	~VirtualFilesystem();

	virtual const char *GetFilesystemType() override { return "vfs"; }
	virtual Capability  Capabilities() override { return Capability(0); }

public:
	// Mount mounts the filesystem at the given path.
	//
	// Negative errno is returned on error.
	//   -EEXIST  If the path already have a mount.
	int Mount(const char *where, const char *source, Filesystem *fs);

	// Umount unmounts a filesystem from the given path.
	// The Filesystem* object is deleted.
	//
	// Negative errno is returned on error.
	//   -ENOENT  If the path does not have a mount.
	int Umount(const char *where);

	// Callback for ListMount
	typedef void (*ListMount_Callback)(void *user, const char *where, const char *source, Filesystem *fs);
	// List all the mounts in the system.
	int ListMount(void *user, ListMount_Callback callback);

	// FilesystemOfPath returns the Filesystem on which
	// the path is to be found.
	//
	// If fspath is not NULL, (*fspath) is set to the path
	// of the file inside the underlying filesystem, pointing
	// into the path string.
	//
	// It takes the last part into account. E.g., it returns
	// the root filesystem if called with "/", and the device
	// filesystem if called with "/dev" or "/dev/".
	Filesystem *FilesystemOfPath(const char *path, const char **fspath);

public:
	virtual int Open(const char *path, OpenFile *file) override;
	virtual int Close(const char *path, OpenFile *file) override;

	virtual int Opendir(const char *path, OpenFile *file) override;
	virtual int Readdir(const char *path, void *user, Readdir_Callback callback, OpenFile *file) override;
	virtual int Closedir(const char *path, OpenFile *file) override;

	virtual uintptr_t Ioctl(const char *path, uintptr_t cmd, void *arg, OpenFile *file) override;

private:
	struct __VirtualFilesystem_Mount {
		runtime::String where, source;
		Filesystem     *fs;
	};
	struct __VirtualFilesystem_OpenFile {
		Filesystem          *fs;       // Underlying filesystem.
		runtime::String      fspath;   // Filesystem path. Allocated.
		Filesystem::OpenFile userfile; // Underlying OpenFile struct, passed to the underlying filesystem.
	};

	// Allocates a new __VFS_OpenFile structure.
	// It does not call OpenXXX() of the underlying FS.
	__VirtualFilesystem_OpenFile *__CreateOpenFile(const char *path);

private:
	__VirtualFilesystem_Mount root;
	vector_Vector            *mounts;
};

// The global virtual filesystem.
extern VirtualFilesystem *VFS;


} // namespace filesystem
} // namespace helos
