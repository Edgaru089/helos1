
#include "vfs.hpp"
#include "string.h"
#include "../../../interrupt/interrupt.h"

namespace helos {
namespace filesystem {


VirtualFilesystem *VFS;

VirtualFilesystem::VirtualFilesystem() {
	mounts = vector_Create(sizeof(void *));
}

VirtualFilesystem::~VirtualFilesystem() {
	for (int i = 0; i < vector_Size(mounts); i++) {
		delete (*((__VirtualFilesystem_Mount **)vector_At(mounts, i)))->fs;
		delete *((__VirtualFilesystem_Mount **)vector_At(mounts, i));
	}
	if (root.fs)
		delete root.fs;
	vector_Destroy(mounts);
}

int VirtualFilesystem::Mount(const char *where, const char *source, Filesystem *fs) {
	size_t len = strlen(where);
	while (len > 1 && where[len - 1] == Seperator) // ditch the trailing seperators
		len--;

	__VirtualFilesystem_Mount *m = new __VirtualFilesystem_Mount;
	m->where                     = runtime::String(where, len);
	m->source                    = source;

	INTERRUPT_DISABLE;
	for (int i = 0; i < vector_Size(mounts); i++)
		if ((*(__VirtualFilesystem_Mount **)vector_At(mounts, i))->where == m->where) {
			delete m;
			INTERRUPT_RESTORE;
			return -EEXIST;
		}

	vector_Push(mounts, &m);
	INTERRUPT_RESTORE;
	return 0;
}

int VirtualFilesystem::Umount(const char *where) {
	return -ENOSYS;
}

Filesystem *VirtualFilesystem::FilesystemOfPath(const char *path, const char **fspath) {
	Filesystem *fs     = root.fs;
	size_t      maxlen = 1;

	INTERRUPT_DISABLE;
	for (int i = 0; i < vector_Size(mounts); i++) {
		__VirtualFilesystem_Mount *m = (__VirtualFilesystem_Mount *)vector_At(mounts, i);
		if (strncmp(m->where.C(), path, m->where.Length()) == 0 && m->where.Length() > maxlen) {
			maxlen = m->where.Length();
			fs     = m->fs;
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
	file->fspath = fspath;

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

	if (!(vfsfile->fs->Capabilities() | Capability_NoOpen)) {
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
