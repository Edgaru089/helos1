#pragma once

#include "../main.h"
#include "efiprot.h"


#ifdef __cplusplus
extern "C" {
#endif

// This defines a default target display mode for graphics_Init().
#define HELOS_GRAPHICS_TARGET_MODE_WIDTH  1600
#define HELOS_GRAPHICS_TARGET_MODE_HEIGHT 900

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

typedef struct {
	int Width, Height;
	int PixelsPerLine;

	EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
} HelosGraphics_Mode;


extern void *   graphics_DeviceFramebuffer; // this is the framebuffer directly for the device via memory mapping.
extern void *   graphics_Framebuffer;       // this is the double-buffered framebuffer (back buffer)
extern uint64_t graphics_FramebufferSize;


// Init() must be called prior to ExitBootServices()
void graphics_Init();


// bitsPerPixel does not count the alpha bits, i.e., is 24 on most modern monitors
void graphics_GetSize(int *sizeX, int *sizeY, int *bitsPerPixel);
void graphics_ClearBuffer(const HelosGraphics_Color *color);
void graphics_SwapBuffer();


// graphics_SetPixel is set by Init() to match one of SetPixel_RGB/BGR according to the framebuffer format.
typedef void(graphics_SetPixel_Type)(int posX, int posY, const HelosGraphics_Color *color);
extern graphics_SetPixel_Type *graphics_SetPixel;

// graphics_SetPixel_RGB/BGR writes the given pixel to the framebuffer in RGB/BGR format.
void graphics_SetPixel_RGB(int posX, int posY, const HelosGraphics_Color *color);
void graphics_SetPixel_BGR(int posX, int posY, const HelosGraphics_Color *color);


void graphics_FillPixel(int startX, int startY, int endX, int endY, const HelosGraphics_Color *color);


extern int graphics_CursorX, graphics_CursorY;

// graphics_Scroll scrolls the display vertically by scrollY pixels.
void graphics_Scroll(int scrollY);
// graphics_ElementSize handles size of an graphics element: scrolling, line breaking, etc.
//
// It does not change CursorX/Y, however.
void graphics_ElementSize(int sizeX, int sizeY);
void graphics_Newline(int advanceY);


void console_WriteChar(const HelosGraphics_Color *color, uint32_t c);
void console_Write(const HelosGraphics_Color *color, const uint32_t *str, int len);
void console_WriteUTF16(const HelosGraphics_Color *color, const uint16_t *str, int len);
void console_WriteASCII(const HelosGraphics_Color *color, const char *str, int len);


#ifdef __cplusplus
} // extern "C"
#endif
