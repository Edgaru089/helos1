#pragma once

#include "../main.h"

#ifdef __cplusplus
extern "C" {
#endif


// HelosGraphics_Color is in ARGB little-endian packed format
// (B,G,R,A in byte order)
typedef struct {
	uint8_t B, G, R;
	uint8_t A;
} PACKED HelosGraphics_Color;

extern const HelosGraphics_Color
	HelosGraphics_Color_Black,
	HelosGraphics_Color_White,
	HelosGraphics_Color_Red,
	HelosGraphics_Color_Green,
	HelosGraphics_Color_Blue,
	HelosGraphics_Color_Cyan,
	HelosGraphics_Color_Magenta,
	HelosGraphics_Color_Yellow;


#ifdef __cplusplus
}
#endif
