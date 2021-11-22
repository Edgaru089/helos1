#pragma once

#include "../filesystem.hpp"
#include "../../block/blockdevice.hpp"
#include "../../../cppruntime/string.hpp"
#include "../../../cppruntime/vector.hpp"

namespace helos {
namespace filesystem {


// DeviceFilesystem is a folder for mounting block and character devices.
// It is usually mounted to /dev.
class DeviceFilesystem: public Filesystem {
public:
	DeviceFilesystem();
	~DeviceFilesystem();
	virtual const char *GetFilesystemType() override { return "devfs"; }
	virtual Capability  Capabilities() override { return Capability_NoOpen; }

public:
	// OpenFile* file might be NULL.
	typedef uintptr_t (*IoctlCallback)(const char *path, void *user, uintptr_t cmd, void *arg, OpenFile *file);

	// Mounts a character device at the given name, with only Ioctl() operations.
	int MountCharacterIoctl(const char *path, void *user, IoctlCallback ioctl);

	// Mounts a block device at the given name.
	int MountBlock(const char *path, block::BlockDevice *block);

	// Unmounts a device at the given path.
	int Umount(const char *path);

public:
	virtual int       Readdir(const char *path, void *user, Readdir_Callback callback, OpenFile *file) override;
	virtual uintptr_t Ioctl(const char *path, uintptr_t cmd, void *arg, OpenFile *file) override;

private:
	struct __DeviceFilesystem_Mount {
		runtime::String path; // begin with '/'
		IoctlCallback   ioctl;
		void           *user;
		mode_t          type; // Either S_IFCHR or S_IFBLK
	};

	// Try to mount a new device. Returns negative errno on error.
	int __InsertMount(__DeviceFilesystem_Mount *m);

private:
	runtime::Vector<__DeviceFilesystem_Mount *> mounts;
};

// The global device filesystem, mounted to /dev
extern DeviceFilesystem *DevFS;


} // namespace filesystem
} // namespace helos
