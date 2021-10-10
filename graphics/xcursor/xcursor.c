
#include "xcursor.h"
#include <string.h>


void xcursor_LoadMemory(xcursor_Xcursor *cursor, void *file, uintptr_t fileSize) {
	cursor->header       = 0;
	xcursor_Header *head = file;

	if (memcmp(head->magic, XCURSOR_MAGIC, XCURSOR_MAGIC_SIZE) != 0)
		return; // magic mismatch

	cursor->size = fileSize;
	cursor->n    = head->numTOC;
	cursor->toc  = file + head->headerSize;

	cursor->header = head;
}
