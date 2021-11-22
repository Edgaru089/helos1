
#include "vfs.hpp"
#include "string.h"

namespace helos {
namespace filesystem {


struct __VirtualFilesystem_Mount {
	char       *where;
	size_t      len;
	Filesystem *fs;
};

VirtualFilesystem::VirtualFilesystem(): root(nullptr) {
	mounts = vector_Create(sizeof(__VirtualFilesystem_Mount));
}

VirtualFilesystem::~VirtualFilesystem() {
	for (int i = 0; i < vector_Size(mounts); i++) {
		delete ((__VirtualFilesystem_Mount *)vector_At(mounts, i))->fs;
		delete[]((__VirtualFilesystem_Mount *)vector_At(mounts, i))->where;
	}
	if (root)
		delete root;
	vector_Destroy(mounts);
}

int VirtualFilesystem::Mount(const char *where, Filesystem *fs) {
	size_t len = strlen(where);
	while (len > 1 && where[len - 1] == Seperator) // ditch the trailing seperators
		len--;

	__VirtualFilesystem_Mount m;
	m.fs    = fs;
	m.len   = len;
	m.where = new char[len + 1];
	memcpy(m.where, where, len);
	m.where[len] = 0;

	for (int i = 0; i < vector_Size(mounts); i++)
		if (strcmp(((__VirtualFilesystem_Mount *)vector_At(mounts, i))->where, m.where) == 0) {
			delete[] m.where;
			return -EEXIST;
		}

	vector_Push(mounts, &m);
	return 0;
}

int VirtualFilesystem::Umount(const char *where) {
	return -ENOSYS;
}

Filesystem *VirtualFilesystem::FilesystemOfPath(const char *path) {
	Filesystem *fs     = root;
	size_t      maxlen = 1;

	for (int i = 0; i < vector_Size(mounts); i++) {
		__VirtualFilesystem_Mount *m = (__VirtualFilesystem_Mount *)vector_At(mounts, i);
		if (strncmp(m->where, path, m->len) == 0 && m->len > maxlen) {
			maxlen = m->len;
			fs     = m->fs;
		}
	}

	return fs;
}


struct __VirtualFilesystem_OpenFile {
	Filesystem          *fs;       // Underlying filesystem.
	Filesystem::OpenFile userfile; // Underlying OpenFile struct, passed to the underlying filesystem.
};


} // namespace filesystem
} // namespace helos
