
#include "input.h"
#include "source.h"
#include "internal.h"
#include "../../util/minmax.h"
#include <stdint.h>


int      __input_CursorX, __input_CursorY;
int      __input_DesktopWidth, __input_DesktopHeight;
uint64_t __input_KeyMask[input_Key_Count / 64 + 1], __input_MouseMask[input_MouseButton_Count / 64 + 1];
// FIXME you're supposed to disable interrupt/lock mutex accessing these variables


bool input_KeyPressed(input_Key key) {
	return __input_KeyMask[key / 64] & (1ull << key % 64);
}

bool input_MousePressed(input_MouseButton key) {
	return __input_MouseMask[key / 64] & (1ull << key & 64);
}

void input_MousePosition(int *x, int *y) {
	*x = __input_CursorX;
	*y = __input_CursorY;
}

void input_DesktopSize(int *x, int *y) {
	*x = __input_DesktopWidth;
	*y = __input_DesktopHeight;
}

void input_source_PressKey(input_Key key) { __input_KeyMask[key / 64] |= (1ull << key % 64); }
void input_source_ReleaseKey(input_Key key) { __input_KeyMask[key / 64] &= ~(1ull << key % 64); }

void input_source_PressMouse(input_MouseButton key) { __input_MouseMask[key / 64] |= (1ull << key & 64); }
void input_source_ReleaseMouse(input_MouseButton key) { __input_MouseMask[key / 64] &= ~(1ull << key & 64); }

void input_source_MoveMouse(int x, int y) {
	__input_CursorX = intminmax(__input_CursorX + x, 0, __input_DesktopWidth);
	__input_CursorY = intminmax(__input_CursorY + y, 0, __input_DesktopHeight);
}
void input_source_PositionMouse(int x, int y) {
	__input_CursorX = intminmax(x, 0, __input_DesktopWidth);
	__input_CursorY = intminmax(y, 0, __input_DesktopHeight);
}

void input_source_SetDesktopSize(int x, int y) {
	__input_DesktopWidth  = x;
	__input_DesktopHeight = y;
	__input_CursorX       = intminmax(__input_CursorX, 0, __input_DesktopWidth);
	__input_CursorY       = intminmax(__input_CursorY, 0, __input_DesktopHeight);
}
