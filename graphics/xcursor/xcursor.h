#pragma once

#include "../../main.h"
#include "../color.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// XCursor file format - see man page xcursor(3)
// https://www.x.org/releases/X11R7.7/doc/man/man3/Xcursor.3.xhtml

#define XCURSOR_MAGIC      "Xcur"
#define XCURSOR_MAGIC_SIZE 4

// Header
typedef struct {
	char     magic[4];   // Magic string "Xcur"
	uint32_t headerSize; // Size of the header (16)
	uint32_t version;    // File version number
	uint32_t numTOC;     // Number of entries in the Table of Contents
} PACKED xcursor_Header;

// Table of Content Entry
typedef struct {
	uint32_t type;    // Entry chunk type (0xfffe0001 = Comment, 0xfffd0002 = Image)
	uint32_t subtype; // Type specific Subtype, size(width=height) for images
	uint32_t offset;  // Absolute byte position in the file
} PACKED xcursor_TOCEntry;


#define XCURSOR_CHUNKTYPE_COMMENT 0xfffe0001u
#define XCURSOR_CHUNKTYPE_IMAGE   0xfffd0002u

// Common parts in different types of Chunk Headers
typedef struct {
	uint32_t headerSize; // Size of the header
	uint32_t type;       // Type of the Chunk, matches the Entry Type in the TOC
	uint32_t subtype;    // Type specific subtype
	uint32_t version;    // Version number of the chunk type
} PACKED xcursor_ChunkHeader;

#define XCURSOR_COMMENT_SUBTYPE_COPYRIGHT 0x00000001u
#define XCURSOR_COMMENT_SUBTYPE_LICENSE   0x00000002u
#define XCURSOR_COMMENT_SUBTYPE_OTHER     0x00000003u

// Chunk Header for type Comment
typedef struct {
	uint32_t headerSize; // Size of the header
	uint32_t type;       // Type of the Chunk, matches the Entry Type in the TOC
	uint32_t subtype;    // Type specific subtype, Copyright, License or Other
	uint32_t version;    // Version number of the chunk type, =1

	uint32_t length;    // Length in bytes of the UTF-8 string
	char     string[1]; // The UTF-8 string, spanning the rest of the chunk
} PACKED xcursor_ChunkHeader_Comment;

typedef struct {
	uint32_t headerSize; // Size of the header
	uint32_t type;       // Type of the Chunk, matches the Entry Type in the TOC
	uint32_t subtype;    // Type specific subtype, Image size
	uint32_t version;    // Version number of the chunk type, =1

	uint32_t            width, height; // Width/Height, <=0x7fff
	uint32_t            xhot, yhot;    // X/Y hotpoint, <=Width/Height
	uint32_t            delay;         // Delay between animation frames in milliseconds
	HelosGraphics_Color pixels[1];     // Packed ARGB little-endian format pixels, with A at the highest byte (BGRA in byte order)
} PACKED xcursor_ChunkHeader_Image;


typedef struct {
	xcursor_Header *  header;
	uintptr_t         size; // size of the file in bytes
	uintptr_t         n;    // number of Chunks/TOC Entries
	xcursor_TOCEntry *toc;  // array of TOC Entries
} xcursor_Xcursor;

// cursor->header is 0 if the open failed.
void xcursor_LoadMemory(xcursor_Xcursor *cursor, void *file, uintptr_t fileSize);


#ifdef __cplusplus
}
#endif
