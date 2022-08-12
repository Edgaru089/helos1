#pragma once

#include <stdint.h>


enum {
	devfs_Framebuffer_ColorFormat_BGR = 0, // b[0]=Blue, b[1]=Green, b[2]=Red
};

typedef struct {
	uint16_t Width;        // Width of the framebuffer, in pixels
	uint16_t Height;       // Height of the framebuffer, in pixels
	uint8_t  BitsPerPixel; // Bits per pixel, must be a multiple of 8
	uint8_t  ColorFormat;  // Color format, one of devfs_Framebuffer_ColorFormat_XXX
} devfs_Framebuffer_Stat;

// Ioctl(GetStat, &out_stat)
const uintptr_t devfs_Framebuffer_Ioctl_GetStat = 0;
