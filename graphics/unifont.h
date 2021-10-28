#pragma once

#include "../main.h"
#include "graphics.h"

#include <stdbool.h>


#define UNIFONT_MAX_CHAR   (0xffff)
#define UNIFONT_CHAR_COUNT (0xffff + 1)

#define UNIFONT_CHAR_WIDTH  8
#define UNIFONT_CHAR_HEIGHT 16

extern const unsigned char unifont_Data[], unifont_Width[];

static inline bool unifont_IsCharDoublewidth(uint32_t ucs) {
	return (ucs >= 0x1100 &&                                      /* formatting in clang-format is a nightmare */
			(ucs <= 0x115f ||                                     /* Hangul Jamo init. consonants */
			 ucs == 0x2329 || ucs == 0x232a ||                    /* Left/Right-Pointing Angle Bracket */
			 (ucs >= 0x2e80 && ucs <= 0xa4cf && ucs != 0x303f) || /* CJK ... Yi */
			 (ucs >= 0xac00 && ucs <= 0xd7a3) ||                  /* Hangul Syllables */
			 (ucs >= 0xf900 && ucs <= 0xfaff) ||                  /* CJK Compatibility Ideographs */
			 (ucs >= 0xfe10 && ucs <= 0xfe19) ||                  /* Vertical forms */
			 (ucs >= 0xfe30 && ucs <= 0xfe6f) ||                  /* CJK Compatibility Forms */
			 (ucs >= 0xff00 && ucs <= 0xff60) ||                  /* Fullwidth Forms */
			 (ucs >= 0xffe0 && ucs <= 0xffe6) ||                  /* formatting in clang-format is a literal nightmare */
			 (ucs >= 0x20000 && ucs <= 0x2fffd) ||                /* one can only use comments like this to control line breaking */
			 (ucs >= 0x30000 && ucs <= 0x3fffd)));
}

void unifont_DrawChar(int posX, int posY, const HelosGraphics_Color *color, uint32_t codepoint);
void unifont_DrawString(int posX, int posY, const HelosGraphics_Color *color, const uint32_t *codepoints, int count);
void unifont_DrawStringUTF16(int posX, int posY, const HelosGraphics_Color *color, const uint16_t *codepoints, int count);
void unifont_DrawStringASCII(int posX, int posY, const HelosGraphics_Color *color, const char *codepoints, int count);
