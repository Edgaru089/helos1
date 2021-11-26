
#include "vfs.hpp"
#include "string.h"
#include "../../../interrupt/interrupt.h"
#include "../../../runtime/stdio.h"

namespace helos {
namespace filesystem {


VirtualFilesystem *VFS;

VirtualFilesystem::VirtualFilesystem() {}

VirtualFilesystem::~VirtualFilesystem() {
	for (int i = 0; i < mounts.Size(); i++) {
		delete mounts[i]->fs;
		delete mounts[i];
	}
	if (root.fs)
		delete root.fs;
}

int VirtualFilesystem::Mount(const char *where, const char *source, Filesystem *fs) {
	if (!fs) {
		io_Errorf("VFS::Mount(): mount to \"%s\" from \"%s\" failed: %s\n", where, source, "nullptr filesystem");
		return -EINVAL;
	}

	if (strcmp(where, "/") == 0) {
		INTERRUPT_DISABLE;
		root.fs     = fs;
		root.source = source;
		root.where  = where;
		INTERRUPT_RESTORE;
		return 0;
	}

	size_t len = strlen(where);
	while (len > 1 && where[len - 1] == Seperator) // ditch the trailing seperators
		len--;

	__VirtualFilesystem_Mount *m = new __VirtualFilesystem_Mount;
	m->fs                        = fs;
	m->where                     = runtime::String(where, len);
	m->source                    = source;

	INTERRUPT_DISABLE;
	for (int i = 0; i < mounts.Size(); i++)
		if (mounts[i]->where == m->where) {
			delete m;
			INTERRUPT_RESTORE;
			return -EEXIST;
		}

	mounts.Push(m);
	INTERRUPT_RESTORE;
	return 0;
}

int VirtualFilesystem::Umount(const char *where) {
	return -ENOSYS;
}

Filesystem *VirtualFilesystem::FilesystemOfPath(const char *path, const char **fspath) {
	if (strcmp(path, "/") == 0) {
		if (fspath)
			(*fspath) = path;
		return root.fs;
	}

	Filesystem *fs     = root.fs;
	size_t      maxlen = 1;

	INTERRUPT_DISABLE;
	for (int i = 0; i < mounts.Size(); i++) {
		__VirtualFilesystem_Mount *m = mounts[i];
		if (strncmp(m->where.C(), path, m->where.Length()) == 0 && m->where.Length() > maxlen) {
			maxlen = m->where.Length();
			fs     = m->fs;
			break;
		}
	}
	INTERRUPT_RESTORE;

	if (fspath) {
		if (fs == root.fs)
			(*fspath) = path; // path==fspath on root filesystem
		else
			(*fspath) = path + maxlen; // fspath should begin with '/'
	}

	return fs;
}


VirtualFilesystem::__VirtualFilesystem_OpenFile *VirtualFilesystem::__CreateOpenFile(const char *path) {
	__VirtualFilesystem_OpenFile *file = new __VirtualFilesystem_OpenFile;
	const char                   *fspath;
	file->fs     = FilesystemOfPath(path, &fspath);
	file->fspath = ((strlen(fspath) == 0) ? "/" : fspath);

	return file;
}

int VirtualFilesystem::Open(const char *path, VirtualFilesystem::OpenFile *file) {
	__VirtualFilesystem_OpenFile *vfsfile = __CreateOpenFile(path);

	if (!(vfsfile->fs->Capabilities() | Capability_NoOpen)) {
		int userret = vfsfile->fs->Open(vfsfile->fspath.C(), &vfsfile->userfile);
		if (userret < 0) {
			delete vfsfile;
			return userret;
		}
	}

	file->handle = (uint64_t)vfsfile;
	return 0;
}
int VirtualFilesystem::Close(const char *path, VirtualFilesystem::OpenFile *file) {
	__VirtualFilesystem_OpenFile *vfsfile = (__VirtualFilesystem_OpenFile *)file->handle;

	int userret = 0;
	if (!(vfsfile->fs->Capabilities() | Capability_NoOpen))
		userret = vfsfile->fs->Close(vfsfile->fspath.C(), &vfsfile->userfile);
	delete vfsfile;
	return userret;
}


int VirtualFilesystem::Opendir(const char *path, VirtualFilesystem::OpenFile *file) {
	__VirtualFilesystem_OpenFile *vfsfile = __CreateOpenFile(path);

	if (!(vfsfile->fs->Capabilities() & Capability_NoOpen)) {
		int userret = vfsfile->fs->Opendir(vfsfile->fspath.C(), &vfsfile->userfile);
		if (userret < 0) {
			delete vfsfile;
			return userret;
		}
	}

	file->handle = (uint64_t)vfsfile;
	return 0;
}
int VirtualFilesystem::Readdir(const char *path, void *user, Readdir_Callback callback, OpenFile *file) {
	__VirtualFilesystem_OpenFile *vfsfile = (__VirtualFilesystem_OpenFile *)file->handle;
	return vfsfile->fs->Readdir(vfsfile->fspath.C(), user, callback, &vfsfile->userfile);
}
int VirtualFilesystem::Closedir(const char *path, OpenFile *file) {
	__VirtualFilesystem_OpenFile *vfsfile = (__VirtualFilesystem_OpenFile *)file->handle;

	int userret = 0;
	if (!(vfsfile->fs->Capabilities() | Capability_NoOpen))
		userret = vfsfile->fs->Closedir(vfsfile->fspath.C(), &vfsfile->userfile);
	delete vfsfile;
	return userret;
}

uintptr_t VirtualFilesystem::Ioctl(const char *path, uintptr_t cmd, void *arg, OpenFile *file) {
	__VirtualFilesystem_OpenFile *vfsfile = (__VirtualFilesystem_OpenFile *)file->handle;
	return vfsfile->fs->Ioctl(vfsfile->fspath.C(), cmd, arg, &vfsfile->userfile);
}

} // namespace filesystem
} // namespace helos
