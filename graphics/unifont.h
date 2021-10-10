#pragma once

#include "../main.h"
#include "graphics.h"

#include <stdbool.h>


#define UNIFONT_MAX_CHAR   (0xffff)
#define UNIFONT_CHAR_COUNT (0xffff + 1)

#define UNIFONT_CHAR_WIDTH  8
#define UNIFONT_CHAR_HEIGHT 16

extern const unsigned char unifont_Data[], unifont_Width[];
extern const unsigned char unifont_Data_End[], unifont_Width_End[]; // Past-the-end pointers for the data files

static inline bool unifont_IsCharDoublewidth(uint32_t codepoint) {
	const unsigned char *ptr = unifont_Width + codepoint / 8;
	if (ptr < unifont_Width_End)
		return (*ptr) & (1u << (7 - codepoint % 8));
	else
		return false;
}

void unifont_DrawChar(int posX, int posY, const HelosGraphics_Color *color, uint32_t codepoint);
void unifont_DrawString(int posX, int posY, const HelosGraphics_Color *color, const uint32_t *codepoints, int count);
void unifont_DrawStringUTF16(int posX, int posY, const HelosGraphics_Color *color, const uint16_t *codepoints, int count);
void unifont_DrawStringASCII(int posX, int posY, const HelosGraphics_Color *color, const char *codepoints, int count);
