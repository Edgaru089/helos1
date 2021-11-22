
#include "filesystem_init.h"
#include "filesystem.hpp"
#include "vfs/vfs.hpp"
#include "devfs/devfs.hpp"
#include "devfs/framebuffer.hpp"
#include "../../runtime/stdio.h"


extern "C" void filesystem_Init() {
	helos::filesystem::VFS   = new helos::filesystem::VirtualFilesystem;
	helos::filesystem::DevFS = new helos::filesystem::DeviceFilesystem;

	int ret = helos::filesystem::VFS->Mount("/dev", "devdefault", helos::filesystem::DevFS);
	if (ret != 0) {
		io_Printf("filesystem_Init(): Mount /dev error: %d\n", ret);
		return;
	}

	helos::filesystem::device::Framebuffer::Init(3);
}

static int __readdir_i = 0;

static int __readdir(void *user, const char *name, const helos::filesystem::Filesystem::Stat *stat, helos::filesystem::Filesystem::Readdir_CallbackFlags flags) {
	io_WriteConsoleASCII(" ");
	io_WriteConsoleASCII(name);
	return 0;
}

extern "C" void filesystem_Ls(const char *path) {
	io_Printf("filesystem_Ls(\"%s\"): ", path);
	helos::filesystem::Filesystem::OpenFile open;

	int ret = helos::filesystem::VFS->Opendir(path, &open);
	if (ret != 0 || (!open.handle)) {
		io_Printf("filesystem_Ls(\"%s\"): helos::filesystem::VFS->Opendir() return error %d\n", ret);
		return;
	}
	ret = helos::filesystem::VFS->Readdir(path, nullptr, __readdir, &open);
	if (ret != 0) {
		io_Printf("filesystem_Ls(\"%s\"): helos::filesystem::VFS->Opendir() return error %d\n", ret);
	}

	io_WriteConsoleASCII("\n");
	helos::filesystem::VFS->Closedir(path, &open);
	return;
}
