
#include "framebuffer.hpp"
#include "../vfs/vfs.hpp"
#include "devfs.hpp"
#include "../../../runtime/panic_assert.h"
#include "../../../runtime/stdio.h"

namespace helos {
namespace filesystem {
namespace device {


namespace {
uintptr_t __Framebuffer_Ioctl(const char *path, void *user, uintptr_t cmd, void *arg, DeviceFilesystem::OpenFile *file) {
	return -Filesystem::ENOSYS;
}
} // namespace

void Framebuffer::Init(int fbN) {
	assert(VFS && "Framebuffer::Init(): VFS not initialized");
	assert(DevFS && "Framebuffer::Init(): DeviceFS not initialized");

	int ret = DevFS->MountCharacterIoctl("/fb0", (void *)0, __Framebuffer_Ioctl);
	if (ret != 0) {
		io_Printf("Framebuffer::Init(): mount /fb0 error: %d\n", ret);
	}

	for (int i = 1; i <= fbN; i++) {
		char buf[12];
		sprintf(buf, "/fb%d", i);
		int ret = DevFS->MountCharacterIoctl(buf, (void *)(uintptr_t)i, __Framebuffer_Ioctl);
		if (ret != 0) {
			io_Printf("Framebuffer::Init(): mount /fb%d error: %d\n", i, ret);
		}
	}
}


} // namespace device
} // namespace filesystem
} // namespace helos
