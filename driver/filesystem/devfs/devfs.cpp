
#include "devfs.hpp"
#include "../../../interrupt/interrupt.h"
#include "../../../runtime/stdio.h"

namespace helos {
namespace filesystem {


DeviceFilesystem *DevFS;

DeviceFilesystem::DeviceFilesystem() {}

DeviceFilesystem::~DeviceFilesystem() {
	for (int i = 0; i < mounts.Size(); i++)
		delete mounts[i];
}


int DeviceFilesystem::__InsertMount(DeviceFilesystem::__DeviceFilesystem_Mount *m) {
	INTERRUPT_DISABLE;
	for (int i = 0; i < mounts.Size(); i++)
		if (mounts[i]->path == m->path) {
			INTERRUPT_RESTORE;
			delete m;
			return -EEXIST;
		}
	mounts.Push(m);
	INTERRUPT_RESTORE;
	return 0;
}

int DeviceFilesystem::MountCharacterIoctl(const char *path, void *user, IoctlCallback ioctl) {
	return __InsertMount(new __DeviceFilesystem_Mount{runtime::String(path), ioctl, user, S_IFCHR});
}

int DeviceFilesystem::MountBlock(const char *path, block::BlockDevice *block) {
	// TODO DeviceFilesystem::MountBlock
	return -ENOSYS;
}

int DeviceFilesystem::Umount(const char *path) {
	return -ENOSYS;
}


int DeviceFilesystem::Readdir(const char *path, void *user, Readdir_Callback callback, OpenFile *file) {
	if (strcmp(path, "/") != 0)
		return -ENOENT;

	Stat stat = {};
	INTERRUPT_DISABLE;
	for (int i = 0; i < mounts.Size(); i++) {
		stat.Mode = mounts[i]->type | 0666;
		callback(user, mounts[i]->path.C() + 1, &stat, Readdir_CallbackFlags_StatValid);
	}
	INTERRUPT_RESTORE;

	return 0;
}

uintptr_t DeviceFilesystem::Ioctl(const char *path, uintptr_t cmd, void *arg, OpenFile *file) {
	INTERRUPT_DISABLE;
	for (int i = 0; i < mounts.Size(); i++)
		if (mounts[i]->path == path) {
			INTERRUPT_RESTORE;
			mounts[i]->ioctl(path, mounts[i]->user, cmd, arg, file);
		}
	INTERRUPT_RESTORE;
	return -ENOENT;
}


} // namespace filesystem
} // namespace helos
