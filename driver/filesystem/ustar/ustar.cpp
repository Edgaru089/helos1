
#include "ustar.hpp"
#include "internal.hpp"
#include "../../../runtime/stdio.h"
#include <string.h>

namespace helos {
namespace filesystem {


namespace {
uint64_t decodeOctets(const char *str, size_t len) {
	if (len == 0)
		len = strlen(str);
	uint64_t v = 0;
	for (int i = 0; i < len; i++)
		v = v * 8 + str[i] - '0';
	return v;
}

template<uint64_t blockSize>
uint64_t roundupBlocks(uint64_t size) {
	return (size + blockSize - 1) / blockSize;
}

const char *
	filetypeName(uint8_t type) {
	switch (type) {
		case __USTAR_Type_NormalFile:
			return "(-) Normal file";
		case __USTAR_Type_Directory:
			return "(d) Directory";
		case __USTAR_Type_BlockDevice:
			return "(b) Block Device";
		case __USTAR_Type_CharacterDevice:
			return "(c) Chatacter Device";
		case __USTAR_Type_FIFO:
			return "(p) Pipe/FIFO";
		case __USTAR_Type_SymbolicLink:
			return "(l) Symbolic link";
	};
	return "(unknown)";
}
} // namespace


USTAR::~USTAR() {
	for (int i = 0; i < files.Size(); i++)
		if (files[i].filename)
			delete[] files[i].filename;
}

Filesystem *USTAR::Allocator::Allocate(const char *source, Config *config) { return nullptr; }

Filesystem *USTAR::Allocator::AllocateBlock(block::BlockDevice *block, Config *config) {
	USTAR *ustar = new USTAR;
	ustar->block = block;
	char buf[512];

	for (int i = 0; i < block->Size();) {
		block->ReadBlock(i, buf, 1);
		__USTAR_Metadata *meta = (__USTAR_Metadata *)buf;
		if (strcmp(meta->magic, __USTAR_Magic) != 0 || strncmp(meta->version, __USTAR_Version, sizeof(__USTAR_Version) - 1) != 0) {
			if (i == 0) {
				delete ustar;
				return nullptr; // no "ustar" magic
			} else
				break;
		}

		size_t filelen = strlen(meta->filename), prefixlen = strlen(meta->prefix);

		__USTAR_File file;
		file.filename    = new char[filelen + prefixlen + 2];
		file.filename[0] = Seperator;
		memcpy(file.filename + 1, meta->prefix, prefixlen);
		strcpy(file.filename + prefixlen + 1, meta->filename);
		file.linkname   = nullptr;
		file.owner      = (uint16_t)decodeOctets(meta->owner, 0);
		file.group      = (uint16_t)decodeOctets(meta->group, 0);
		file.lastmodify = decodeOctets(meta->lastmodify, 0);
		file.size       = 0;
		file.offset     = i + 1;

		// Get the mode_t
		file.mode = ((Filesystem::mode_t)decodeOctets(meta->filemode, 0)) & 0777;
		switch (meta->type) {
			case 0:
			case __USTAR_Type_NormalFile:
				file.mode &= Filesystem::S_IFREG;
				file.size = decodeOctets(meta->size, 0); // Only normal files have size
				break;
			case __USTAR_Type_HardLink:
				// Hard link not supported
				break;
			case __USTAR_Type_SymbolicLink: {
				file.mode &= Filesystem::S_IFLNK;
				size_t linklen = strlen(meta->linkname);
				file.linkname  = new char[linklen + 1];
				memcpy(file.linkname, meta->linkname, linklen + 1);
				break;
			}
			case __USTAR_Type_CharacterDevice:
				file.mode &= Filesystem::S_IFCHR;
				break;
			case __USTAR_Type_BlockDevice:
				file.mode &= Filesystem::S_IFBLK;
				break;
			case __USTAR_Type_Directory:
				file.mode &= Filesystem::S_IFDIR;
				break;
			case __USTAR_Type_FIFO:
				file.mode &= Filesystem::S_IFIFO;
		}

		io_Printf("USTAR::AllocateBlock(): filename=%s, filetype=%s, fileSize=%llu (%llu blocks)\n", file.filename, filetypeName(meta->type), file.size, roundupBlocks<512>(file.size));

		ustar->files.Push(file);
		i += roundupBlocks<512>(file.size) + 1;
	}

	return ustar;
}


void USTAR::__Stat(Stat *stat, int i) {
	stat->Size             = files[i].size;
	stat->Blocks           = roundupBlocks<512>(files[i].size);
	stat->Gid              = files[i].group;
	stat->Uid              = files[i].owner;
	stat->Mode             = files[i].mode;
	stat->NumLinks         = 0;
	stat->TimeModification = files[i].lastmodify;
	stat->TimeAccess       = 0;
}

int USTAR::Getattr(const char *path, Stat *stat, OpenFile *file) {
	if (strcmp(path, "/") == 0) {
		stat->Mode = S_IFDIR | 0755;
		return 0;
	}

	for (int i = 0; i < files.Size(); i++)
		if (strcmp(files[i].filename, path) == 0) {
			__Stat(stat, i);
			return 0;
		}
	return -ENOENT;
}

int USTAR::Readlink(const char *link, char *buffer, uint64_t bufferSize) {
	for (int i = 0; i < files.Size(); i++)
		if (strcmp(files[i].filename, link) == 0) {
			if (!files[i].linkname || (files[i].mode & S_IFMT) != S_IFLNK)
				return -EINVAL; // Not a symlink

			uint64_t linklen = strlen(files[i].linkname);
			if (bufferSize < linklen) {
				memcpy(buffer, files[i].linkname, bufferSize);
				return linklen;
			} else
				memcpy(buffer, files[i].linkname, (bufferSize > linklen) ? linklen + 1 : bufferSize);

			return 0;
		}
	return -ENOENT;
}


int USTAR::Open(const char *path, OpenFile *file) {
	for (int i = 0; i < files.Size(); i++)
		if (strcmp(files[i].filename, path) == 0) {
			if ((files[i].mode & S_IFMT) == S_IFLNK || (files[i].mode & S_IFMT) == S_IFDIR)
				return -EINVAL;
			file->handle = i;
			return 0;
		}
	return -ENOENT;
}

int USTAR::Read(const char *path, char *buffer, uint64_t count, uint64_t offset, OpenFile *file) {
	char          buf[512];
	__USTAR_File &f        = files[file->handle];
	uint64_t      realsize = (f.size - offset < count) ? f.size - offset : count;
	uint64_t      bufoff   = 0;
	for (int i = offset / 512; i <= (offset + realsize - 1) / 512; i++) {
		block->ReadBlock(i, buf, 0);
		memcpy(buffer + bufoff, buf + offset % 512, 512 - offset % 512);
		bufoff = (offset / 512 + 1) * 512 - offset;
		offset += bufoff;
	}
	return realsize;
}

int USTAR::Close(const char *path, OpenFile *file) {
	return 0;
}


int USTAR::Opendir(const char *path, OpenFile *file) {
	if (strcmp(path, "/") == 0) {
		file->handle = files.Size();
		return 0;
	}

	for (int i = 0; i < files.Size(); i++)
		if (strcmp(files[i].filename, path) == 0) {
			if ((files[i].mode & S_IFMT) != S_IFDIR)
				return -EINVAL;
			file->handle = i;
			return 0;
		}
	return -ENOENT;
}

int USTAR::Readdir(const char *path, void *user, Readdir_Callback callback, OpenFile *file) {
	if (strcmp(path, "/") == 0)
		path++;
	size_t pathlen = strlen(path);

	for (int i = 0; i < files.Size(); i++) {
		if (i == file->handle || strncmp(files[i].filename, path, pathlen) != 0 || files[i].filename[pathlen] != Seperator)
			continue; // Not an ancestor

		size_t curlen = strlen(files[i].filename);
		for (int i = pathlen + 1; i < curlen; i++)
			if (files[i].filename[i] == Seperator)
				continue; // Not directly a child

		// OK!
		Stat stat;
		__Stat(&stat, i);
		callback(user, files[i].filename + pathlen + 1, &stat, Readdir_CallbackFlags_StatValid);
	}
	return 0;
}

int USTAR::Closedir(const char *path, OpenFile *file) {
	return 0;
}


} // namespace filesystem
} // namespace helos
