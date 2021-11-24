#pragma once

#include "../filesystem.hpp"
#include "../../../cppruntime/vector.hpp"

namespace helos {
namespace filesystem {


// Internal USTAR header struct
struct __USTAR_File;

// USTAR is a read-only TAR filesystem driver.
class USTAR: public Filesystem {
public:
	USTAR() {}
	~USTAR();

	virtual const char *GetFilesystemType() override { return "ustar"; }
	virtual Capability  Capabilities() override { return Capability(0); }

public:
	// Create a new Filesystem instance from a given source.
	virtual Filesystem *Allocate(const char *source, Config *config) override;
	// Create a new Filesystem instance from a Block Device.
	virtual Filesystem *AllocateBlock(block::BlockDevice *block, Config *config) override;

public:
	// Operations we do not support
	virtual int Mknod(const char *file, mode_t mode) override { return -EROFS; }
	virtual int Unlink(const char *path) override { return -EROFS; }
	virtual int Rmdir(const char *path) override { return -EROFS; }
	virtual int Symlink(const char *target, const char *path) override { return -EROFS; }
	virtual int Rename(const char *from, const char *to, RenameType type) override { return -EROFS; }
	virtual int Link(const char *from, const char *to) override { return -EROFS; }
	virtual int Chown(const char *path, uid_t uid, gid_t gid, OpenFile *file) override { return -EROFS; }
	virtual int Chmod(const char *path, mode_t mode, OpenFile *file) override { return -EROFS; }
	virtual int Truncate(const char *path, uint64_t size, OpenFile *file) override { return -EROFS; }
	virtual int Write(const char *path, const char *buffer, uint64_t count, uint64_t offset, OpenFile *file) override { return -EROFS; }

public:
	// Stat a file.
	virtual int Getattr(const char *path, Stat *stat, OpenFile *file) override;
	// Read a symbolic link.
	virtual int Readlink(const char *link, char *buffer, uint64_t bufferSize) override;

	// Open a file.
	virtual int Open(const char *path, OpenFile *file) override;
	// Read data from an open file.
	virtual int Read(const char *path, char *buffer, uint64_t count, uint64_t offset, OpenFile *file) override;
	// Close an open file.
	virtual int Close(const char *path, OpenFile *file) override;

	// Open a directory for reading.
	virtual int Opendir(const char *path, OpenFile *file) override;
	// Read an open directory.
	virtual int Readdir(const char *path, void *user, Readdir_Callback callback, OpenFile *file) override;
	// Close an open directory.
	virtual int Closedir(const char *path, OpenFile *file) override;

private:
	runtime::Vector<__USTAR_File> files; // Vector holding all the files in the archive.
};


} // namespace filesystem
} // namespace helos
