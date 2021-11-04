
#include "fat.hpp"
#include <string.h>

namespace helos {
namespace filesystem {


FAT::~FAT() {
	if (sector)
		delete[](char *) sector;
	if (bpb)
		delete bpb;

	// either ebr or ebr32
	if (ebr) {
		if (type == FAT32)
			delete ebr32;
		else
			delete ebr;
	}
}

Filesystem *FAT::AllocateBlock(BlockDevice *block, FAT::Config *config) {
	FAT *fat    = new FAT;
	fat->sector = new char[512];

	// read the first sector
	uint64_t read = block->ReadBlock(0, fat->sector, 1);
	if (read == 0) {
		delete fat;
		return nullptr;
	}

	// allocate the BPB
	fat->bpb = new BIOSParamBlock;
	memcpy(fat->bpb, fat->sector, sizeof(BIOSParamBlock));
	fat->numSectors = ((fat->bpb->numSectors_short != 0) ? fat->bpb->numSectors_short : fat->bpb->numSectors_large);

	// tell the filesystem FAT type
	if (fat->bpb->numBytesPerSector == 0) { // exFAT
		fat->type = exFAT;
		delete fat;
		return nullptr; // TODO exFAT is not supported

	} else if (fat->bpb->numSectorsPerFAT_12_16 == 0) { // FAT32
		fat->type  = FAT32;
		fat->ebr32 = new ExtBootRecord_32;
		memcpy(fat->ebr32, (char *)fat->sector + sizeof(BIOSParamBlock), sizeof(ExtBootRecord_32));

	} else { // FAT12 or 16
		fat->ebr = new ExtBootRecord_12_16;
		memcpy(fat->ebr, (char *)fat->sector + sizeof(BIOSParamBlock), sizeof(ExtBootRecord_12_16));

		// Let's tell FAT12 from FAT16
		// see fat12_fat16.txt
		//   "For disks containing more than 4085 clusters (note that
		//    4085 is the correct number), a 16-bit FAT entry is used."
		//
		// Let's first try the name
		if (strncmp(fat->ebr->type, "FAT12", 5) == 0)
			fat->type = FAT12;
		else if (strncmp(fat->ebr->type, "FAT16", 5) == 0)
			fat->type = FAT16;
		// Try the number of clusters
		else {
			int clusters = fat->numSectors / fat->bpb->numSectorsPerCluster;
			if (fat->numSectors % fat->bpb->numSectorsPerCluster != 0)
				clusters++; // round up
			if (clusters > 4085)
				fat->type = FAT16;
			else
				fat->type = FAT12;
		}
	}

	// read the FAT

	return fat;
}


} // namespace filesystem
} // namespace helos
