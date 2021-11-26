
#include "filesystem_init.h"
#include "filesystem.hpp"
#include "format.hpp"
#include "vfs/vfs.hpp"
#include "devfs/devfs.hpp"
#include "devfs/framebuffer.hpp"
#include "ustar/ustar.hpp"

#include "../block/blockdevice.hpp"
#include "../block/ramdisk.hpp"

#include "../../runtime/stdio.h"
#include "../../embed/files.h"

using namespace helos;
using namespace helos::filesystem;


namespace {

template<int sectorSize, int stepBytes = 16>
void hexdump(block::BlockDevice *block, int sector) {
	char buf[sectorSize];
	if (block->ReadBlock(sector, buf, 1))
		return;
	for (int i = 0; i < sectorSize; i += stepBytes) {
		io_Printf("%04x:", i);
		for (int j = 0; j < stepBytes; j += 2)
			io_Printf(" %02x%02x", (uintptr_t)buf[j], (uintptr_t)buf[j + 1]);
		io_WriteConsoleASCII("\n");
	}
}

} // namespace

extern "C" void filesystem_Init() {
	// Register all the filesystems.
	Format::Register(new USTAR::Allocator);

	VFS = new VirtualFilesystem;

	uintptr_t initrdSize        = embed_Initrd_End - embed_Initrd;
	uintptr_t initrdSizeRounded = ((initrdSize % 512) ? (initrdSize / 512 + 1) * 512 : initrdSize);

	block::BlockDevice *initrd = new block::BlockDeviceRamdisk((void *)embed_Initrd, 512, initrdSizeRounded / 512, PermRead);
	Filesystem         *initfs = Format::AllocateBlock(initrd, nullptr);
	if (VFS->Mount("/", "initrd", initfs) < 0) {
		if (initfs)
			delete initfs;
		delete initrd;
	}

	DevFS   = new DeviceFilesystem;
	int ret = VFS->Mount("/dev", "devdefault", DevFS);
	if (ret != 0) {
		io_Printf("filesystem_Init(): Mount /dev error: %d\n", ret);
		return;
	}

	device::Framebuffer::Init(3);
}

static int __readdir_i = 0;

static int __readdir(void *user, const char *name, const Filesystem::Stat *stat, Filesystem::Readdir_CallbackFlags flags) {
	io_WriteConsoleASCII(" ");
	io_WriteConsoleASCII(name);
	return 0;
}

extern "C" void filesystem_Ls(const char *path) {
	io_Printf("filesystem_Ls(\"%s\"): ", path);
	Filesystem::OpenFile open;

	int ret = VFS->Opendir(path, &open);
	if (ret != 0 || (!open.handle)) {
		io_Printf("filesystem_Ls(\"%s\"): filesystem::VFS->Opendir() return error %d\n", ret);
		return;
	}
	ret = VFS->Readdir(path, nullptr, __readdir, &open);
	if (ret != 0) {
		io_Printf("filesystem_Ls(\"%s\"): filesystem::VFS->Opendir() return error %d\n", ret);
	}

	io_WriteConsoleASCII("\n");
	VFS->Closedir(path, &open);
	return;
}
