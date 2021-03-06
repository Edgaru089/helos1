#pragma once

#include "../main.h"

#include "color.h"
#include "xcursor/xcursor.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	graphics_PixelFormat_RGBA_8bit,
	graphics_PixelFormat_BGRA_8bit,
} graphics_PixelFormat;

typedef struct {
	int Width, Height;
	int PixelsPerLine;

	graphics_PixelFormat PixelFormat;
} HelosGraphics_Mode;


extern void              *graphics_DeviceFramebuffer; // this is the framebuffer directly for the device via memory mapping.
extern void              *graphics_Framebuffer;       // this is the double-buffered framebuffer (back buffer)
extern uint64_t           graphics_FramebufferSize;
extern HelosGraphics_Mode graphics_SystemVideoMode; // system video mode

extern xcursor_ChunkHeader_Image *graphics_Cursor; // mouse cursor image


// Init() must be called prior to ExitBootServices()
void graphics_Init();


// bitsPerPixel does not count the alpha bits, i.e., is 24 on most modern monitors
void graphics_GetSize(int *sizeX, int *sizeY, int *bitsPerPixel);
void graphics_ClearBuffer(const HelosGraphics_Color *color);
void graphics_SwapBuffer();
void graphics_Invalidate(int left, int top, int width, int height); // Invalidates a rectangular region


// graphics_SetPixel_RGB/BGR writes the given pixel to the framebuffer in RGB/BGR format.
void graphics_SetPixel_RGB(int posX, int posY, const HelosGraphics_Color *color);
void graphics_SetPixel_BGR(int posX, int posY, const HelosGraphics_Color *color);

// graphics_SetPixel calls one of SetPixel_RGB/BGR according to the framebuffer format.
static inline void graphics_SetPixel(int posX, int posY, const HelosGraphics_Color *color) {
	if (graphics_SystemVideoMode.PixelFormat == graphics_PixelFormat_BGRA_8bit)
		graphics_SetPixel_BGR(posX, posY, color);
	else if (graphics_SystemVideoMode.PixelFormat == graphics_PixelFormat_RGBA_8bit)
		graphics_SetPixel_RGB(posX, posY, color);
}


void graphics_FillPixel(int startX, int startY, int endX, int endY, const HelosGraphics_Color *color);


extern int graphics_CursorX, graphics_CursorY;

// graphics_Scroll scrolls the display vertically by scrollY pixels.
void graphics_Scroll(int scrollY);
// graphics_ElementSize handles size of an graphics element: scrolling, line breaking, etc.
//
// It does not change CursorX/Y, however. Neither does it call Invalidate().
void graphics_ElementSize(int sizeX, int sizeY);
void graphics_Newline(int advanceY);


void console_WriteChar(const HelosGraphics_Color *color, uint32_t c);
void console_Write(const HelosGraphics_Color *color, const uint32_t *str, int len);
void console_WriteUTF16(const HelosGraphics_Color *color, const uint16_t *str, int len);
void console_WriteASCII(const HelosGraphics_Color *color, const char *str, int len);


#ifdef __cplusplus
} // extern "C"
#endif
