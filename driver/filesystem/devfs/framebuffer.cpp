
#include "framebuffer.hpp"
#include "../vfs/vfs.hpp"
#include "devfs.hpp"
#include "../../../runtime/panic_assert.h"
#include "../../../runtime/stdio.h"

#include "../../../graphics/graphics.h"
#include "../../../include/devfs/framebuffer.h"

namespace helos {
namespace filesystem {
namespace device {


namespace {
uintptr_t __Framebuffer_Ioctl(const char *path, void *user, uintptr_t cmd, void *arg, DeviceFilesystem::OpenFile *file) {
	switch (cmd) {
		case devfs_Framebuffer_Ioctl_GetStat:
			if (user == 0) {
				devfs_Framebuffer_Stat stat = {
					.Width        = (uint16_t)(graphics_SystemVideoMode.Width),
					.Height       = (uint16_t)(graphics_SystemVideoMode.Height),
					.BitsPerPixel = 4,
					.ColorFormat  = devfs_Framebuffer_ColorFormat_BGR,
				};
				memcpy(arg, &stat, sizeof(stat));
				return 0;
			} else {
				return -Filesystem::EINVAL;
			}
			break;
		default:
			return -Filesystem::EINVAL;
	}
}

uintptr_t __Framebuffer_Write(const char *path, void *user, const char *buffer, uint64_t count, uint64_t offset, DeviceFilesystem::OpenFile *file) {
	if (offset % 4 != 0 || count % 4 != 0)
		return -Filesystem::EINVAL;

	int xy = offset / 4;
	if (xy > graphics_SystemVideoMode.Width * graphics_SystemVideoMode.Height)
		xy = graphics_SystemVideoMode.Width * graphics_SystemVideoMode.Height;
	int xyend = (offset + count) / 4;
	if (xyend > graphics_SystemVideoMode.Width * graphics_SystemVideoMode.Height)
		xyend = graphics_SystemVideoMode.Width * graphics_SystemVideoMode.Height;

	memcpy((char *)(graphics_Framebuffer) + xy * 4, buffer, (xyend - xy) * 4);
	return (xyend - xy) * 4;
}

uintptr_t __Framebuffer_Read(const char *path, void *user, char *buffer, uint64_t count, uint64_t offset, DeviceFilesystem::OpenFile *file) {
	return -Filesystem::ENOSYS;
}
} // namespace

void Framebuffer::Init(int fbN) {
	assert(VFS && "Framebuffer::Init(): VFS not initialized");
	assert(DevFS && "Framebuffer::Init(): DeviceFS not initialized");

	int ret = DevFS->MountCharacter("/fb0", (void *)0, __Framebuffer_Read, __Framebuffer_Write, __Framebuffer_Ioctl);
	if (ret != 0) {
		io_Printf("Framebuffer::Init(): mount /fb0 error: %d\n", ret);
	}

	for (int i = 1; i <= fbN; i++) {
		char buf[12];
		sprintf(buf, "/fb%d", i);
		int ret = DevFS->MountCharacter("/fb0", (void *)i, __Framebuffer_Read, __Framebuffer_Write, __Framebuffer_Ioctl);
		if (ret != 0) {
			io_Printf("Framebuffer::Init(): mount /fb%d error: %d\n", i, ret);
		}
	}
}


} // namespace device
} // namespace filesystem
} // namespace helos
