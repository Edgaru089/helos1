#pragma once

#include "consts.h"
#include "stdint.h"


extern int      __input_CursorX, __input_CursorY;
extern int      __input_DesktopWidth, __input_DesktopHeight;
extern uint64_t __input_KeyMask[input_Key_Count / 64 + 1], __input_MouseMask[input_MouseButton_Count / 64 + 1]; // bitsets for keys and mouse buttons
