#pragma once

#include "../../main.h"
#include "../../cppruntime/runtime.hpp"

namespace helos {
namespace block {


// BlockDevice describes random-access storage devices organizing data in blocks of uniform sizes.
class BlockDevice {
public:
	virtual ~BlockDevice() {}

	// BlockSize returns the size, in bytes, of the data block.
	// It is to remain constant throughout the lifetime of the device.
	virtual uint64_t BlockSize() const = 0;

	// Size returns the size, in blocks, of the entire device.
	virtual uint64_t Size() const = 0;

	// SizeBytes returns the size of the entire device in bytes.
	virtual uint64_t SizeBytes() const {
		return BlockSize() * Size();
	}


	// Permission returns the permission on the entire device (Read, Write, Exec)
	virtual Permission DevicePermission() const {
		return PermRead | PermWrite | PermExecute;
	}

	// ReadBlock reads, from the block device, blockCount blocks of data into the buffer.
	//
	// Returns the number of blocks read, or a negative number on error.
	virtual uint64_t ReadBlock(uint64_t blockOffset, void *data, uint64_t blockCount) const = 0;

	// WriteBlock writes into the block device blockCount blocks.
	//
	// Returns the number of blocks written, or a negative number on error.
	// (no data is written on error)
	virtual uint64_t WriteBlock(uint64_t blockOffset, const void *data, uint64_t blockCount) = 0;


public:
	// Slice creates a new BlockDevice reading from a portion of the underlying device.
	// The underlying device must be kept alive as long as the slice exists.
	BlockDevice *Slice(uint64_t blockOffset, uint64_t blockCount);
};


} // namespace block
} // namespace helos
