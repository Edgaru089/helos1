#pragma once

// This file is intended only for input source devices like a PS/2 driver.

#include <stdbool.h>
#include "consts.h"

#ifdef __cplusplus
extern "C" {
#endif


void input_source_PressKey(input_Key key);
void input_source_ReleaseKey(input_Key key);

void input_source_PressMouse(input_MouseButton key);
void input_source_ReleaseMouse(input_MouseButton key);

void input_source_MoveMouse(int x, int y);     // Moves mouse cursor by a relative amount
void input_source_PositionMouse(int x, int y); // Moves mouse cursor to an absolute position

// input_source_SetDesktopSize sets the size of the rectangular area to which
// the mouse cursor is bounded.
void input_source_SetDesktopSize(int x, int y);


#ifdef __cplusplus
}
#endif
