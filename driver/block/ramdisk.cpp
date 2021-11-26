
#include "ramdisk.hpp"
#include "../../memory/memory.h"
#include "../../memory/paging_internal.h"
#include <string.h>

namespace helos {
namespace block {


constexpr uint16_t __Ramdisk_Alloc_None    = 0,
				   __Ramdisk_Alloc_Malloc  = 1,
				   __Ramdisk_Alloc_4KPages = 2,
				   __Ramdisk_Alloc_2MPages = 3;

BlockDeviceRamdisk::BlockDeviceRamdisk(uint64_t blockSize, uint64_t blockCount, ::helos::Permission perm)
	: blocksize(blockSize), blockcount(blockCount), perm(perm) {
	uint64_t bytes = blockSize * blockCount; // the number of bytes to allocate

	if (bytes < SYSTEM_PAGE_SIZE) {
		// less than a page: use malloc()
		alloctype = __Ramdisk_Alloc_Malloc;
		buffer    = kMalloc(bytes);
	} else if (bytes < SYSTEM_PAGE_2M_SIZE * 64) {
		// less than 128M: 4K pages
		alloctype = __Ramdisk_Alloc_4KPages;
		buffer    = (void *)memory_AllocateKernelMapping(bytes, SYSTEM_PAGE_SIZE);
		paging_map_PageAllocated((uint64_t)buffer, pages = roundUpToPageCount(bytes), (int)perm);
	} else {
		// more than 128M: 2M pages
		alloctype = __Ramdisk_Alloc_2MPages;
		buffer    = (void *)memory_AllocateKernelMapping(bytes, SYSTEM_PAGE_2M_SIZE);
		paging_map_PageAllocated2M((uint64_t)buffer, pages = roundUpToPageCount2M(bytes), (int)perm);
	}
}

BlockDeviceRamdisk::BlockDeviceRamdisk(void *buffer, uint64_t blockSize, uint64_t blockCount, ::helos ::Permission perm)
	: buffer(buffer), blocksize(blockSize), blockcount(blockCount), perm(perm), alloctype(__Ramdisk_Alloc_None) {}

BlockDeviceRamdisk::~BlockDeviceRamdisk() {
	switch (alloctype) {
		case __Ramdisk_Alloc_Malloc:
			kFree(buffer);
			break;
		case __Ramdisk_Alloc_4KPages:
		case __Ramdisk_Alloc_2MPages:
			paging_map_FreeAllocated(
				(uint64_t)buffer,
				(uint64_t)buffer + pages * ((alloctype == __Ramdisk_Alloc_4KPages) ? SYSTEM_PAGE_SIZE : SYSTEM_PAGE_2M_SIZE));
			break;
	}
}

uint64_t BlockDeviceRamdisk::ReadBlock(uint64_t offset, void *data, uint64_t count) const {
	if (!(perm & PermRead))
		return -1;
	if (count > blockcount - offset)
		count = blockcount - offset;
	memcpy(data, (uint8_t *)buffer + blocksize * offset, blocksize * count);
	return count;
}

uint64_t BlockDeviceRamdisk::WriteBlock(uint64_t offset, const void *data, uint64_t count) {
	if (!(perm & PermWrite))
		return -1;
	if (count > blockcount - offset)
		count = blockcount - offset;
	memcpy((uint8_t *)buffer + blocksize * offset, data, blocksize * count);
	return count;
}


} // namespace block
} // namespace helos
