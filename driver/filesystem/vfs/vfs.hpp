#pragma once

#include "../../../cppruntime/runtime.hpp"
#include "../filesystem.hpp"
#include "../../../util/vector.h"

namespace helos {
namespace filesystem {


// VirtualFilesystem is the global virtual filesystem.
class VirtualFilesystem: public Filesystem {
public:
	VirtualFilesystem();
	~VirtualFilesystem();

public:
	// Mount mounts the filesystem at the given path.
	//
	// Negative errno is returned on error.
	//   -EEXIST  If the path already have a mount.
	int Mount(const char *where, Filesystem *fs);

	// Umount unmounts a filesystem from the given path.
	// The Filesystem* object is deleted.
	//
	// Negative errno is returned on error.
	//   -ENOENT  If the path does not have a mount.
	int Umount(const char *where);

	// FilesystemOfPath returns the Filesystem on which
	// the path is to be found.
	//
	// It takes the last part into account. E.g., it returns
	// the root filesystem if called with "/", and the device
	// filesystem if called with "/dev" or "/dev/".
	Filesystem *FilesystemOfPath(const char *path);

private:
	Filesystem    *root;
	vector_Vector *mounts;
};


} // namespace filesystem
} // namespace helos
