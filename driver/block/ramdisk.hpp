#pragma once

#include "blockdevice.hpp"

namespace helos {


// Ramdisk is a block device residing in runtime RAM.
class BlockDeviceRamdisk: public BlockDevice {
public:
	BlockDeviceRamdisk(const BlockDeviceRamdisk &) = delete; // Don't copy by value
	BlockDeviceRamdisk(BlockDeviceRamdisk &&)      = delete;
	const BlockDeviceRamdisk &operator=(const BlockDeviceRamdisk &) = delete;

public:
	// Creates new, empty (all zero) ramdisk, allocating from kernel memory
	BlockDeviceRamdisk(uint64_t blockSize, uint64_t blockCount, ::helos::Permission perm);
	// Creates a Ramdisk wrapper from an existing buffer
	// Size of the buffer must be a multiple of BlockSize
	BlockDeviceRamdisk(void *buffer, uint64_t blockSize, uint64_t blockCount, ::helos ::Permission perm);
	~BlockDeviceRamdisk();

public:
	uint64_t BlockSize() const override { return blocksize; }
	uint64_t Size() const override { return blockcount; }
	uint64_t SizeBytes() const override { return blocksize * blockcount; }

	::helos::Permission DevicePermission() const override { return perm; }

	uint64_t ReadBlock(uint64_t blockOffset, void *data, uint64_t blockCount) const override;
	uint64_t WriteBlock(uint64_t blockOffset, const void *data, uint64_t blockCount) override;

public:
	// Buffer returns the underlying buffer.
	void *Buffer() { return buffer; }

private:
	uint64_t            blocksize, blockcount;
	void *              buffer;
	int                 pages; // Number of pages allocated in 4K or 2M paging allocation
	::helos::Permission perm;
	uint16_t            alloctype; // Type of the buffer allocated
};


} // namespace helos
