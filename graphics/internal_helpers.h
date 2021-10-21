#pragma once

#include "../util/minmax.h"
#include "color.h"
#include "stdint.h"
#include "string.h"


static inline void __graphics_CopyBuffer32(
	void *from,        // Source image of the copy
	int   fromOffsetX, // OffsetX of the region to copy from
	int   fromOffsetY, // OffsetY of the region to copy from
	int   fromSizeX,   // SizeX of the entire source image
	int   fromSizeY,   // SizeY of the entire source image
	void *to,          // Destination image of the copy
	int   toOffsetX,   // OffsetX of the target region
	int   toOffsetY,   // OffsetY of the target region
	int   toSizeX,     // SizeX of the entire destination image
	int   toSizeY,     // SizeY of the entire destination image
	int   countX,      // Size of the copying region
	int   countY) {
	int beginX = intmax3(0, -fromOffsetX, -toOffsetX), beginY = intmax3(0, -fromOffsetY, -toOffsetY);
	int endX = intmin3(countX, fromSizeX - fromOffsetX, toSizeX - toOffsetX),
		endY = intmin3(countY, fromSizeY - fromOffsetY, toSizeY - toOffsetY);

#define PIXEL_BYTES ((intptr_t)4)
	for (int i = beginY; i < endY; i++)
		memcpy(
			((intptr_t)toOffsetX + beginX + (intptr_t)toSizeX * (toOffsetY + i)) * PIXEL_BYTES + (uint8_t *)to,
			((intptr_t)fromOffsetX + beginX + (intptr_t)fromSizeX * (fromOffsetY + i)) * PIXEL_BYTES + (uint8_t *)from,
			(endX - beginX) * PIXEL_BYTES);
#undef PIXEL_BYTES
}

// Blends ARGB Premultiplied Alpha image onto (opaque) buffer
// See: https://apoorvaj.io/alpha-compositing-opengl-blending-and-premultiplied-alpha/
static inline void __graphics_RenderBuffer32(
	void *from,        // Source image of the render
	int   fromOffsetX, // OffsetX of the region to render from
	int   fromOffsetY, // OffsetY of the region to render from
	int   fromSizeX,   // SizeX of the entire source image
	int   fromSizeY,   // SizeY of the entire source image
	void *to,          // Destination image of the render
	int   toOffsetX,   // OffsetX of the target region
	int   toOffsetY,   // OffsetY of the target region
	int   toSizeX,     // SizeX of the entire destination image
	int   toSizeY,     // SizeY of the entire destination image
	int   countX,      // Size of the render region
	int   countY) {
	int beginX = intmax3(0, -fromOffsetX, -toOffsetX), beginY = intmax3(0, -fromOffsetY, -toOffsetY);
	int endX = intmin3(countX, fromSizeX - fromOffsetX, toSizeX - toOffsetX),
		endY = intmin3(countY, fromSizeY - fromOffsetY, toSizeY - toOffsetY);

	for (int x = beginX; x < endX; x++)
		for (int y = beginY; y < endY; y++) {
			HelosGraphics_Color
				*src  = ((intptr_t)fromOffsetX + x + (intptr_t)fromSizeX * (fromOffsetY + y)) + (HelosGraphics_Color *)from,
				*dest = ((intptr_t)toOffsetX + x + (intptr_t)toSizeX * (toOffsetY + y)) + (HelosGraphics_Color *)to;

			if (src->A == 0)
				continue;
			else if (src->A == 0xff)
				*(uint32_t *)dest = *(uint32_t *)src & 0x00ffffffu;
			else { // blend
				HelosGraphics_Color color = {
					.B = (uint8_t)(src->B + ((int)dest->B) * (0xff - src->A) / 0xff),
					.G = (uint8_t)(src->G + ((int)dest->G) * (0xff - src->A) / 0xff),
					.R = (uint8_t)(src->R + ((int)dest->R) * (0xff - src->A) / 0xff),
					.A = 0};
				*dest = color;
			}
		}
}
