
#include "blockdevice.hpp"

namespace helos {


class SlicedBlockDevice: public BlockDevice {
public:
	SlicedBlockDevice(BlockDevice *backing, uint64_t off, uint64_t cnt)
		: backing(backing), off(off), cnt(cnt) {}

public:
	virtual uint64_t BlockSize() const override {
		return backing->BlockSize();
	}

	uint64_t Size() const override {
		return cnt;
	}

	uint64_t SizeBytes() const override {
		return backing->BlockSize() * cnt;
	}

	::helos::Permission Permission() const override {
		return backing->Permission();
	}

	uint64_t ReadBlock(uint64_t blockOffset, void *data, uint64_t blockCount) const override {
		return backing->ReadBlock(blockOffset + off, data, blockCount);
	}

	uint64_t WriteBlock(uint64_t blockOffset, const void *data, uint64_t blockCount) override {
		return backing->WriteBlock(blockOffset + off, data, blockCount);
	}

private:
	BlockDevice *backing;
	uint64_t     off, cnt;
};


BlockDevice *BlockDevice::Slice(uint64_t blockOffset, uint64_t blockCount) {
	return new SlicedBlockDevice(this, blockOffset, blockCount);
}


} // namespace helos
