
#include "format.hpp"

namespace helos {
namespace filesystem {


runtime::Vector<FilesystemAllocator *> *Format::fslist;

void Format::Register(FilesystemAllocator *fs) {
	if (!fslist)
		fslist = new runtime::Vector<FilesystemAllocator *>;

	fslist->Push(fs);
}

Filesystem *Format::Allocate(const char *source, Filesystem::Config *config) {
	Filesystem *fs;
	for (int i = 0; i < fslist->Size(); i++)
		if ((fs = (*fslist)[i]->Allocate(source, config)))
			return fs;
	return nullptr;
}

// Allocate a Filesystem instance from a Block Device.
Filesystem *Format::AllocateBlock(block::BlockDevice *block, Filesystem::Config *config) {
	Filesystem *fs;
	for (int i = 0; i < fslist->Size(); i++)
		if ((fs = (*fslist)[i]->AllocateBlock(block, config)))
			return fs;
	return nullptr;
}


} // namespace filesystem
} // namespace helos
