

#include "graphics.h"
#include "color.h"
#include "internal_helpers.h"
#include "unifont.h"
#include "../runtime/stdio.h"
#include "../util/minmax.h"
#include "../util/stack.h"
#include <efiprot.h>

#include <assert.h>
#include <string.h>
#include "../runtime/memcpy.h"

const HelosGraphics_Color
	HelosGraphics_Color_Black   = {0x00, 0x00, 0x00, 0xff},
	HelosGraphics_Color_White   = {0xff, 0xff, 0xff, 0xff},
	HelosGraphics_Color_Red     = {0x00, 0x00, 0xff, 0xff},
	HelosGraphics_Color_Green   = {0x00, 0xff, 0x00, 0xff},
	HelosGraphics_Color_Blue    = {0xff, 0x00, 0x00, 0xff},
	HelosGraphics_Color_Cyan    = {0xff, 0xff, 0x00, 0xff},
	HelosGraphics_Color_Magenta = {0xff, 0x00, 0xff, 0xff},
	HelosGraphics_Color_Yellow  = {0x00, 0xff, 0xff, 0xff};


static EFI_GUID                      gopID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

static EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
static UINTN                                 sizeofInfo, nModes, nativeMode;
static HelosGraphics_Mode                    modes[128];
HelosGraphics_Mode                           graphics_SystemVideoMode;

void *graphics_DeviceFramebuffer;

void *   graphics_Framebuffer;
uint64_t graphics_FramebufferSize;
uint32_t graphics_Doublebuffer[2048 * 1024];

#define MOUSE_OVERLAY_SIZE 32
xcursor_ChunkHeader_Image *graphics_Cursor;
int                        graphics_MouseCursorX, graphics_MouseCursorY;
static int                 __lastMouseX, __lastMouseY; // Last of mouse **IMAGE** position
static int                 __lastMouseSizeX, __lastMouseSizeY;
static uint32_t            __mouseOverlay[MOUSE_OVERLAY_SIZE * MOUSE_OVERLAY_SIZE];

#define INVALIDATED_BUFFER_SIZE 32
typedef struct {
	uint32_t left, top;
	uint32_t width, height;
} PACKED               __graphics_Rect;
static bool            __invalidated_screen;
static stack *         __invalidated;
static __graphics_Rect __invalidated_buffer[INVALIDATED_BUFFER_SIZE];


void graphics_Init() {
	assert(sizeof(HelosGraphics_Color) == sizeof(uint32_t) && "HelosGraphics_Color not packed to be 32-bit(4 bytes)");

	EFI_STATUS status = efiBootServices->LocateProtocol(&gopID, NULL, (void **)&gop);
	if (EFI_ERROR(status)) {
		io_Printf("graphics_Init: Error locating GOP\r\n");
		return;
	}


	status = gop->QueryMode(gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &sizeofInfo, &info);
	if (status == EFI_NOT_STARTED) {
		status = gop->SetMode(gop, 0);
	}
	if (EFI_ERROR(status)) {
		io_Printf("graphics_Init: Error getting native modes\r\n");
		return;
	} else {
		nativeMode = gop->Mode->Mode;
		nModes     = gop->Mode->MaxMode;
	}

	for (int i = 0; i < nModes; i++) {
		status                 = gop->QueryMode(gop, i, &sizeofInfo, &info);
		modes[i].Width         = info->HorizontalResolution;
		modes[i].Height        = info->VerticalResolution;
		modes[i].PixelsPerLine = info->PixelsPerScanLine;
		modes[i].PixelFormat   = info->PixelFormat;

		if (modes[i].Width == HELOS_GRAPHICS_TARGET_MODE_WIDTH && modes[i].Height == HELOS_GRAPHICS_TARGET_MODE_HEIGHT)
			nativeMode = i;
	}

	// set the new mode
	status = gop->SetMode(gop, nativeMode);
	if (EFI_ERROR(status)) {
		io_Printf("graphics_Init: Unable to set mode %d\r\n", nativeMode);
		return;
	}

	io_Printf(
		"graphics_Init: Framebuffer:\r\n  addr %08x len %d, size %dx%d, ppl %d\r\n",
		graphics_DeviceFramebuffer = (void *)gop->Mode->FrameBufferBase,
		graphics_FramebufferSize   = gop->Mode->FrameBufferSize,
		gop->Mode->Info->HorizontalResolution,
		gop->Mode->Info->VerticalResolution,
		gop->Mode->Info->PixelsPerScanLine);

	// warn the user if the framebuffer is not RGB
	io_Printf("graphics_Init: Framebuffer format: ");
#define CASE(v) \
	break;      \
	case v: io_Printf(#v "\r\n");
	switch (gop->Mode->Info->PixelFormat) {
		CASE(PixelRedGreenBlueReserved8BitPerColor)
		graphics_SetPixel = graphics_SetPixel_RGB;
		CASE(PixelBlueGreenRedReserved8BitPerColor)
		graphics_SetPixel = graphics_SetPixel_BGR;
		CASE(PixelBitMask)
		CASE(PixelBltOnly)
		CASE(PixelFormatMax)
	}
#undef CASE

	graphics_SystemVideoMode = modes[nativeMode];
	graphics_Framebuffer     = (void *)graphics_Doublebuffer;
	graphics_CursorX = graphics_CursorY = 0;

	graphics_MouseCursorX = graphics_SystemVideoMode.Width / 2;
	graphics_MouseCursorY = graphics_SystemVideoMode.Height / 2;

	stack_InitBuffered(__invalidated, __invalidated_buffer, sizeof(__invalidated_buffer));
	__invalidated_screen = false;
}


graphics_SetPixel_Type *graphics_SetPixel;

void graphics_SetPixel_RGB(int posX, int posY, const HelosGraphics_Color *color) {
	struct {
		uint8_t R, G, B;
		uint8_t A;
	} colorRGB = {color->R, color->G, color->B, 0};

	*((uint32_t *)(graphics_Framebuffer + graphics_SystemVideoMode.PixelsPerLine * 4 * posY + 4 * posX)) = (*((uint32_t *)&colorRGB));
}

void graphics_SetPixel_BGR(int posX, int posY, const HelosGraphics_Color *color) {
	*((uint32_t *)(graphics_Framebuffer + graphics_SystemVideoMode.PixelsPerLine * 4 * posY + 4 * posX)) = (*((uint32_t *)color)) & 0x00ffffffu;
}


void graphics_GetSize(int *sizeX, int *sizeY, int *bitsPerPixel) {
	*sizeX        = graphics_SystemVideoMode.Width;
	*sizeY        = graphics_SystemVideoMode.Height;
	*bitsPerPixel = 24;
}

void graphics_ClearBuffer(const HelosGraphics_Color *color) {
	uint32_t data;

	if (*((uint32_t *)color) == *((uint32_t *)&HelosGraphics_Color_Black)) {
		memset(graphics_Framebuffer, 0, graphics_FramebufferSize);
		return;
	}

	if (graphics_SystemVideoMode.PixelFormat == PixelRedGreenBlueReserved8BitPerColor) {
		struct {
			uint8_t R, G, B;
			uint8_t A;
		} colorRGB = {color->R, color->G, color->B, 0};
		data       = (*(uint32_t *)&colorRGB) & 0x00ffffffu;
	} else if (graphics_SystemVideoMode.PixelFormat == PixelBlueGreenRedReserved8BitPerColor) {
		data = (*(uint32_t *)color) & 0x00ffffffu;
	}

	uint32_t *buffer = graphics_Framebuffer, *end = graphics_Framebuffer + graphics_FramebufferSize / sizeof(uint32_t);
	while (buffer != end) {
		*buffer = data;
		buffer++;
	}
}

static void __graphics__UpdateMouse() {
	if (!graphics_Cursor)
		return;

	int imgX = graphics_MouseCursorX - graphics_Cursor->xhot + 1;
	int imgY = graphics_MouseCursorY - graphics_Cursor->yhot + 1;
	if (imgX != __lastMouseX || imgY != __lastMouseY || graphics_Cursor->width != __lastMouseSizeX || graphics_Cursor->height != __lastMouseSizeY) { // moved
		__graphics_CopyBuffer32(
			__mouseOverlay, 0, 0, MOUSE_OVERLAY_SIZE, MOUSE_OVERLAY_SIZE,
			graphics_Framebuffer, __lastMouseX, __lastMouseY, graphics_SystemVideoMode.Width, graphics_SystemVideoMode.Height,
			__lastMouseSizeX, __lastMouseSizeY);
		__graphics_CopyBuffer32(
			graphics_Framebuffer, imgX, imgY, graphics_SystemVideoMode.Width, graphics_SystemVideoMode.Height,
			__mouseOverlay, 0, 0, MOUSE_OVERLAY_SIZE, MOUSE_OVERLAY_SIZE,
			graphics_Cursor->width, graphics_Cursor->height);
		__graphics_RenderBuffer32(
			graphics_Cursor->pixels, 0, 0, graphics_Cursor->width, graphics_Cursor->height,
			graphics_Framebuffer, imgX, imgY, graphics_SystemVideoMode.Width, graphics_SystemVideoMode.Height,
			graphics_Cursor->width, graphics_Cursor->height);
		graphics_Invalidate(__lastMouseX, __lastMouseY, __lastMouseSizeX, __lastMouseSizeY);
		graphics_Invalidate(imgX, imgY, graphics_Cursor->width, graphics_Cursor->height);
		__lastMouseX     = imgX;
		__lastMouseY     = imgY;
		__lastMouseSizeX = graphics_Cursor->width;
		__lastMouseSizeY = graphics_Cursor->height;
	}
}

void graphics_SwapBuffer() {
	__graphics__UpdateMouse();
	if (__invalidated_screen)
		memcpy(graphics_DeviceFramebuffer, graphics_Framebuffer, graphics_FramebufferSize);
	else {
		__graphics_Rect r;
		while (stack_Pop(__invalidated, &r, sizeof(__graphics_Rect))) {
			__graphics_CopyBuffer32(
				graphics_Framebuffer,
				r.left, r.top,
				graphics_SystemVideoMode.Width, graphics_SystemVideoMode.Height,
				graphics_DeviceFramebuffer,
				r.left, r.top,
				graphics_SystemVideoMode.Width, graphics_SystemVideoMode.Height,
				r.width, r.height);
		}
	}
	stack_Clear(__invalidated);
}

void graphics_FillPixel(int startX, int startY, int endX, int endY, const HelosGraphics_Color *color) {
	// TODO Optimize this! This is too sloooow
	if (graphics_SystemVideoMode.PixelFormat == PixelBlueGreenRedReserved8BitPerColor)
		for (int i = startX; i < endX; i++)
			for (int j = startY; j < endY; j++)
				*((uint32_t *)(graphics_Framebuffer + graphics_SystemVideoMode.PixelsPerLine * 4 * j + 4 * i)) = (*((uint32_t *)color)) & 0x00ffffffu;
	else if (graphics_SystemVideoMode.PixelFormat == PixelRedGreenBlueReserved8BitPerColor)
		for (int i = startX; i < endX; i++)
			for (int j = startY; j < endY; j++) {
				struct {
					uint8_t B, G, R;
					uint8_t A;
				} colorRGB = {color->B, color->G, color->R, 0};

				*((uint32_t *)(graphics_Framebuffer + graphics_SystemVideoMode.PixelsPerLine * 4 * j + 4 * i)) = (*((uint32_t *)&colorRGB));
			}
	graphics_Invalidate(startX, startY, endX - startX, endY - startY);
}

void graphics_Invalidate(int left, int top, int width, int height) {
	if (stack_Space(__invalidated) < sizeof(__graphics_Rect)) {
		// No more space - invalidate the entire screen
		__invalidated_screen = true;
		return;
	}
	__graphics_Rect prev, r = {left, top, width, height};
	// Try to merge the new rect with the last one
#define POPOLD_SETNEW(a, b, c, d)                                 \
	{                                                             \
		r.left   = a;                                             \
		r.top    = b;                                             \
		r.width  = c;                                             \
		r.height = d;                                             \
		stack_Pop(__invalidated, &prev, sizeof(__graphics_Rect)); \
	}
	if (stack_Top(__invalidated, &prev, sizeof(__graphics_Rect)) != 0) {
		if (prev.top == r.top && prev.height == r.height) {
			if (prev.left + prev.width == r.left)
				POPOLD_SETNEW(prev.left, r.top, prev.width + r.width, r.height)
			else if (r.left + r.width == prev.left)
				POPOLD_SETNEW(r.left, r.top, r.width + prev.width, r.height)
		} else if (prev.left == r.left && prev.width == r.width) {
			if (prev.top + prev.height == r.top)
				POPOLD_SETNEW(r.left, prev.top, r.width, prev.height + r.height)
			else if (r.top + r.height == prev.top)
				POPOLD_SETNEW(r.left, r.top, r.width, r.height + prev.height);
		}
	}
	stack_Push(__invalidated, &r, sizeof(__graphics_Rect));
#undef POPOLD_SETNEW
}

int graphics_CursorX, graphics_CursorY;

void graphics_Scroll(int scrollY) {
	memmove(
		graphics_Doublebuffer,
		graphics_Doublebuffer + graphics_SystemVideoMode.PixelsPerLine * scrollY,
		sizeof(uint32_t) * (graphics_SystemVideoMode.PixelsPerLine * (graphics_SystemVideoMode.Height - scrollY)));
	// TODO proper memset instead of this sloooow FillPixel
	/*memset(
		graphics_Doublebuffer + graphics_SystemVideoMode.PixelsPerLine * (graphics_SystemVideoMode.Height - scrollY),
		0,
		sizeof(uint32_t) * graphics_SystemVideoMode.PixelsPerLine * (scrollY));*/
	graphics_FillPixel(0, graphics_SystemVideoMode.Height - scrollY, graphics_SystemVideoMode.Width, graphics_SystemVideoMode.Height, &HelosGraphics_Color_Black);
	__lastMouseY -= scrollY;
	__invalidated_screen = true; // invalidates the whole screen
}

void graphics_ElementSize(int sizeX, int sizeY) {
	if (graphics_CursorX + sizeX >= graphics_SystemVideoMode.Width) { // line breaking required
		graphics_CursorY += sizeY;
		graphics_CursorX = 0;
	}
	if (graphics_CursorY + sizeY >= graphics_SystemVideoMode.Height) { // scrolling required
		graphics_Scroll(sizeY + graphics_CursorY - graphics_SystemVideoMode.Height);
		graphics_CursorY = graphics_SystemVideoMode.Height - sizeY;
	}
}

void graphics_Newline(int advanceY) {
	graphics_CursorY += advanceY;
	graphics_CursorX = 0;
}


void console_WriteChar(const HelosGraphics_Color *color, uint32_t c) {
	int width;
	switch (c) {
		case '\n':
			graphics_Newline(UNIFONT_CHAR_HEIGHT);
			break;
		case '\r':
			graphics_CursorX = 0;
			break;
		default:
			width = unifont_IsCharDoublewidth(c) ? UNIFONT_CHAR_WIDTH * 2 : UNIFONT_CHAR_WIDTH;
			graphics_ElementSize(width, UNIFONT_CHAR_HEIGHT);
			graphics_FillPixel(graphics_CursorX, graphics_CursorY, graphics_CursorX + width, graphics_CursorY + UNIFONT_CHAR_HEIGHT, &HelosGraphics_Color_Black);
			unifont_DrawChar(graphics_CursorX, graphics_CursorY, color, c);
			graphics_Invalidate(graphics_CursorX, graphics_CursorY, width, UNIFONT_CHAR_HEIGHT);
			graphics_CursorX += width;
	}
}

void console_Write(const HelosGraphics_Color *color, const uint32_t *str, int len) {
	bool wantSwap = false;
	if (len == 0) {
		while (*str != 0) {
			console_WriteChar(color, *str);
			if (*str == '\n')
				wantSwap = true;
			str++;
		}
	} else {
		for (int i = 0; i < len; i++) {
			console_WriteChar(color, str[i]);
			if (str[i] == '\n')
				wantSwap = true;
		}
	}
	if (wantSwap)
		graphics_SwapBuffer();
}

void console_WriteUTF16(const HelosGraphics_Color *color, const uint16_t *str, int len) {
	bool wantSwap = false;
	if (len == 0) {
		while (*str != 0) {
			console_WriteChar(color, *str);
			if (*str == '\n')
				wantSwap = true;
			str++;
		}
	} else {
		for (int i = 0; i < len; i++) {
			console_WriteChar(color, str[i]);
			if (str[i] == '\n')
				wantSwap = true;
		}
	}
	if (wantSwap)
		graphics_SwapBuffer();
}

void console_WriteASCII(const HelosGraphics_Color *color, const char *str, int len) {
	bool wantSwap = false;
	if (len == 0) {
		while (*str != 0) {
			console_WriteChar(color, *str);
			if (*str == '\n')
				wantSwap = true;
			str++;
		}
	} else {
		for (int i = 0; i < len; i++) {
			console_WriteChar(color, str[i]);
			if (str[i] == '\n')
				wantSwap = true;
		}
	}
	if (wantSwap)
		graphics_SwapBuffer();
}
