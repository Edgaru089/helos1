#pragma once

#include "../main.h"
#include "efiprot.h"

#include "color.h"
#include "xcursor/xcursor.h"

#ifdef __cplusplus
extern "C" {
#endif

// This defines a default target display mode for graphics_Init().
#define HELOS_GRAPHICS_TARGET_MODE_WIDTH  1600
#define HELOS_GRAPHICS_TARGET_MODE_HEIGHT 900


typedef struct {
	int Width, Height;
	int PixelsPerLine;

	EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
} HelosGraphics_Mode;


extern void *             graphics_DeviceFramebuffer; // this is the framebuffer directly for the device via memory mapping.
extern void *             graphics_Framebuffer;       // this is the double-buffered framebuffer (back buffer)
extern uint64_t           graphics_FramebufferSize;
extern HelosGraphics_Mode graphics_SystemVideoMode; // system video mode

extern xcursor_ChunkHeader_Image *graphics_Cursor; // mouse cursor image
extern int                        graphics_MouseCursorX, graphics_MouseCursorY;


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
