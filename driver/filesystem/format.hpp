#pragma once

#include "filesystem.hpp"
#include "../../cppruntime/vector.hpp"

namespace helos {
namespace filesystem {


class FilesystemAllocator {
public:
	// Allocate is called to create a new Filesystem instance from a given source.
	//
	// The source paramater is quite special.
	// Beginning with "/" means a local file source (device or image).
	// Beginning with a "//" means a network target (URL).
	// Otherwise, this string is non-canonical and its behavior is implementation-depedent.
	//
	// On error, NULL is returned.
	virtual Filesystem *Allocate(const char *source, Filesystem::Config *config) { return nullptr; }

	// AllocateBlock is called to create a new Filesystem instance from a Block Device.
	//
	// On error, NULL is returned.
	virtual Filesystem *AllocateBlock(block::BlockDevice *block, Filesystem::Config *config) { return nullptr; }
};

class Format {
public:
	// Register a filesystem format.
	static void Register(FilesystemAllocator *fs);

	// Allocate a Filesystem instance from a source, or NULL if error.
	static Filesystem *Allocate(const char *source, Filesystem::Config *config);

	// Allocate a Filesystem instance from a Block Device.
	static Filesystem *AllocateBlock(block::BlockDevice *block, Filesystem::Config *config);

private:
	static runtime::Vector<FilesystemAllocator *> *fslist;
};


} // namespace filesystem
} // namespace helos
