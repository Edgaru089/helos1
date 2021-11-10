#pragma once

#include "../../smp/condiction.h"
#include <stdbool.h>
#include "consts.h"

#ifdef __cplusplus
extern "C" {
#endif


// If input_Condition is not NULL, it is notified on new input events.
extern smp_Condition *input_Condition;

// input_KeyPressed tells if a given key is present and pressed.
bool input_KeyPressed(input_Key key);

// input_MousePressed tells if a given mouse button is pressed.
bool input_MousePressed(input_MouseButton key);

// input_MouseButton returns the mouse position.
void input_MousePosition(int *x, int *y);

// input_DesktopSize returns the size of the rectangular area to which
// the mouse cursor is bounded.
//
// Don't use this to do anything not related to mouse position, e.g., get the screen size.
void input_DesktopSize(int *x, int *y);


#ifdef __cplusplus
}
#endif
