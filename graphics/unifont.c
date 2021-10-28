
#include "unifont.h"
#include "graphics.h"
#include <stddef.h>

size_t strlen(const char *);

void unifont_DrawChar(int posX, int posY, const HelosGraphics_Color *color, uint32_t codepoint) {
	const unsigned char *data = unifont_Data + codepoint * UNIFONT_CHAR_WIDTH * UNIFONT_CHAR_HEIGHT * 2 / 8;
	bool                 wide = unifont_IsCharDoublewidth(codepoint);

	// HACK Assuming UNIFONT_CHAR_WIDTH is 8
	if (wide) {
		const uint8_t *line = data;
		for (int y = 0; y < UNIFONT_CHAR_HEIGHT; y++) {
			for (int x = 0; x < UNIFONT_CHAR_WIDTH; x++)
				if (*line & (1u << (7 - x % 8)))
					graphics_SetPixel(posX + x, posY + y, color);
			line++;
			for (int x = 0; x < UNIFONT_CHAR_WIDTH; x++)
				if (*line & (1u << (7 - x % 8)))
					graphics_SetPixel(posX + x + 8, posY + y, color);
			line++;
		}
	} else {
		const uint8_t *line = data;
		for (int y = 0; y < UNIFONT_CHAR_HEIGHT; y++, line++) {
			for (int x = 0; x < UNIFONT_CHAR_WIDTH; x++)
				if (*line & (1u << (7 - x % 8)))
					graphics_SetPixel(posX + x, posY + y, color);
		}
	}
}

void unifont_DrawString(int posX, int posY, const HelosGraphics_Color *color, const uint32_t *codepoints, int count) {
	for (const uint32_t *end = codepoints + count; codepoints != end; codepoints++) {
		unifont_DrawChar(posX, posY, color, *codepoints);
		posX += UNIFONT_CHAR_WIDTH * (unifont_IsCharDoublewidth(*codepoints) ? 2 : 1);
	}
}
void unifont_DrawStringUTF16(int posX, int posY, const HelosGraphics_Color *color, const uint16_t *codepoints, int count) {
	for (const uint16_t *end = codepoints + count; codepoints != end; codepoints++) {
		unifont_DrawChar(posX, posY, color, *codepoints);
		posX += UNIFONT_CHAR_WIDTH * (unifont_IsCharDoublewidth(*codepoints) ? 2 : 1);
	}
}
void unifont_DrawStringASCII(int posX, int posY, const HelosGraphics_Color *color, const char *codepoints, int count) {
	if (count == 0) {
		count = strlen(codepoints);
	}
	for (const char *end = codepoints + count; codepoints != end; codepoints++) {
		unifont_DrawChar(posX, posY, color, *codepoints);
		//posX += UNIFONT_CHAR_WIDTH * (unifont_IsCharDoublewidth(*codepoints) ? 2 : 1);
		posX += UNIFONT_CHAR_WIDTH; // visible ASCII chars are all single-width
	}
}
