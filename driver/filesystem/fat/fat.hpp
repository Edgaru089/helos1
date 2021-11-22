#pragma once

#include "../../../cppruntime/runtime.hpp"
#include "../filesystem.hpp"
#include "../../../main.h"

namespace helos {

namespace filesystem {


// FAT is a FAT12/16/32 driver.
class FAT: public Filesystem {
private:
	// Type of the FAT filesystem (FAT12, FAT16 or FAT32)
	enum FATType {
		FAT12,
		FAT16,
		FAT32,
		exFAT,
	};


	// The 1st sector of the FAT filesystem, BIOS Parameter Block and Extended Boot Record
	// Same for FAT12/16 and FAT32.
	struct BIOSParamBlock {
		char     jumpcode[3];            // EB XX 90, a jmp:short XX; nop command
		char     oemid[8];               // OEM identifier, can be any string
		uint16_t numBytesPerSector;      // number of bytes per sector (512)
		uint8_t  numSectorsPerCluster;   // number of sectors per cluster (8, 512*8=4096)
		uint16_t numReservedSector;      // number of reserved sectors, including the BPB
		uint8_t  numFATs;                // number of FATs, often 2
		uint16_t numDirEntry;            // number of directory entries
		uint16_t numSectors_short;       // total sectors in the volume, 0 if the value is too large to fit and in sectorCount_large;
		uint8_t  mediaDescriptorType;    // Media Descriptor Type
		uint16_t numSectorsPerFAT_12_16; // number of sectors per FAT, set only for FAT12/16
		uint16_t numSectorsPerTrack;     // number of sectors per track
		uint16_t numSides;               // number of heads or sides of the media
		uint32_t offsetLBA;              // number of hidden sectors (i.e., the LBA of the beginning of the partition.)
		uint32_t numSectors_large;       // large sector count, set if sectorCount_short is 0
	} PACKED;
	static_assert(sizeof(BIOSParamBlock) == 36, "BIOS Parameter Block struct not packed");


	// Extended Boot Record for FAT12/16, comes right after BPB
	struct ExtBootRecord_12_16 {
		uint8_t  driveNumber; // drive number, i.e., 0x00 for floppy and 0x80 for harddisk. Useless.
		uint8_t  ntFlags;     // flags in Windows NT, reserved and zero.
		uint8_t  signature;   // signature, must be 0x28 or 0x29.
		uint32_t volumeID;    // volume ID (partition UUID)
		char     label[11];   // label, padded with spaces
		char     type[8];     // system identifier string, containing the FAT filesystem type. Not to be trusted.
	} PACKED;
	static_assert(sizeof(ExtBootRecord_12_16) == 62 - 36, "ExtBootRecord_12_16 not packed");


	// Extended Boot Record for FAT32, comes right after BPB
	struct ExtBootRecord_32 {
		uint32_t sectorsPerFAT;        // number of sectors in per FAT
		uint16_t flags;                // flags
		uint16_t version;              // FAT version number. High byte is the major version and low byte is the minor. Can be zero.
		uint32_t clusterRoot;          // the cluster number (offset) of the root directory. Often 2.
		uint16_t sectorFSinfo;         // the sector number of the FSInfo structure
		uint16_t sectorBackupBootsect; // the sector number of the backup boot sector
		char     reserved[12];         // reserved. Should be set to zero on format.

		uint8_t  driveNumber; // drive number, i.e., 0x00 for floppy and 0x80 for harddisk. Useless.
		uint8_t  ntFlags;     // flags in Windows NT, reserved and zero.
		uint8_t  signature;   // signature, must be 0x28 or 0x29.
		uint32_t volumeID;    // volume ID (partition UUID)
		char     label[11];   // label, padded with spaces
		char     type[8];     // system identifier string, always "FAT32   " with 3 spaces. Not to be trusted.
	} PACKED;
	static_assert(sizeof(ExtBootRecord_32) == 90 - 36, "ExtBootRecord_36 not packed");


	static constexpr int FSInfo_Offset = 484;
	// FSInfo structure for FAT32, starting at byte offset 484
	struct FSInfo {
		uint32_t signatureMid;      // = 0x61417272
		uint32_t freeClusterCount;  // last known free cluster count, might be incorrect. 0xFFFFFFFF means the value is unknown and must be computed.
		uint32_t freeClusterOffset; // the cluster number at which the driver should start looking for free clusters. 0xFFFFFFFF means there is no hint and the driver should start at 2. Also might be incorrect.
		char     reserved[12];      // reserved
		uint32_t signatureTrail;    // = 0xAA550000
	} PACKED;
	static_assert(sizeof(FSInfo) == 512 - 484, "FSInfo_off484 not packed");

	static constexpr uint32_t
		FSInfo_SignatureLead  = 0x41615252, // Lead signature at the beginning of the FSInfo sector
		FSInto_SignatureMid   = 0x61417272, // Signature at offset 0x1e4
		FSInfo_SignatureTrail = 0xaa550000; // Trail signature at the end of the sector


	// Time in a Directory Entry
	struct DirEntry_Time {
		uint16_t hours : 5;        // Hours in wall time.
		uint16_t minutes : 6;      // Minutes in wall time.
		uint16_t seconds_div2 : 5; // Seconds in wall time divided by 2 (sec = div2*2 + 10th/100)
	};

	// Date in a Directory Entry
	struct DirEntry_Date {
		uint16_t year_1980 : 7; // Calendar year, offset since 1980
		uint16_t month : 4;     // Calendar month, 1-12
		uint16_t day : 5;       // Calendar day, 1-31
	};

	// Directory Entry
	struct DirEntry {
		char          name[8], ext[3];         // File name and extension (standard 8.3 format)
		uint8_t       flags;                   // Flags of the file.
		char          reserved;                // Reserved by Windows NT.
		uint8_t       createTime_Seconds_10th; // Tenths of a second, range 0-199 inclusive.
		DirEntry_Time createTime;              // Create time
		DirEntry_Date createDate;              // Create time
		DirEntry_Date lastAccessDate;          // Last access date. there are no last-access time.
		uint16_t      firstCluster_high;       // High 16 bits of the first cluster number.
		DirEntry_Time modifyTime;              // Last modify time
		DirEntry_Date modifyDate;              // Last modify date
		uint16_t      firstCluster_low;        // Low 16 bits of the first cluster number.
		uint32_t      size;                    // Size of the file in bytes
	} PACKED;
	static_assert(sizeof(DirEntry) == 32, "DirEntry not packed");

	static constexpr uint8_t
		DirEntry_Flags_ReadOnly              = 0x01, // Read-Only
		DirEntry_Flags_ReadOnly_Hidden       = 0x02, // Hidden
		DirEntry_Flags_ReadOnly_System       = 0x04, // System
		DirEntry_Flags_ReadOnly_VolumeID     = 0x08, // A Volume-ID entry, only in the root folder
		DirEntry_Flags_ReadOnly_Directory    = 0x10, // Directory
		DirEntry_Flags_ReadOnly_Archive      = 0x20, // Archive
		DirEntry_Flags_ReadOnly_LongFileName = 0x0f; // Part of a Long File Name entry

public:
	~FAT();

	// AllocateBlock allocates a new FAT driver from a block device.
	//
	// It only supports 512-byte-block devices.
	virtual Filesystem *AllocateBlock(block::BlockDevice *block, Config *config) override;

public:
	// Opendir opens a new directory.
	//
	// It places the cluster number of the directory into file->handle.
	virtual int Opendir(const char *path, OpenFile *file) override;

	// Readdir reads a whole directory in one go, calling the callback on each element.
	virtual int Readdir(const char *path, void *user, Readdir_Callback callback, OpenFile *file) override;

	// Closedir closes a open directory.
	//
	// Currently it does nothing.
	virtual int Closedir(const char *path, OpenFile *file) override;

private:
	// returns the next cluster number from this one, or 0 if end-of-chain
	int __NextCluster(int cluster);

	// reads a directory from a cluster number
	virtual int __Readdir(int cluster, void *user, Readdir_Callback callback);

private:
	FATType type;

	// A copy of the BPB and EBR
	BIOSParamBlock *bpb;
	union {
		ExtBootRecord_12_16 *ebr;
		ExtBootRecord_32 *   ebr32;
	};

	// A copy of the entire FAT
	// TODO change this! this takes about 1/128 of the disk size in memory (in FAT32)
	void *fat_table;
	void *sector; // A buffer at the size of a sector. Allocated on creation

	int numSectors; // Number of sectors in the entire partition.
};


} // namespace filesystem
} // namespace helos
