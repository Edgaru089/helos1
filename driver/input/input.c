
#include "input.h"
#include "consts.h"
#include "source.h"
#include "internal.h"
#include "../../util/minmax.h"
#include "../../interrupt/interrupt.h"
#include "../../runtime/stdio.h"


smp_Condition *input_Condition;
queue_Queue   *input_EventQueue;
int            __input_CursorX, __input_CursorY;
int            __input_DesktopWidth, __input_DesktopHeight;
uint64_t       __input_KeyMask[input_Key_Count / 64 + 1], __input_MouseMask[input_MouseButton_Count / 64 + 1];
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

static inline void notify(void *data) {
	if (input_Condition)
		smp_Condition_NotifyAll(input_Condition, data);
}

static inline bool flagControl() { return input_KeyPressed(input_Key_LControl) || input_KeyPressed(input_Key_RControl); }
static inline bool flagShift() { return input_KeyPressed(input_Key_LShift) || input_KeyPressed(input_Key_RShift); }
static inline bool flagAlt() { return input_KeyPressed(input_Key_LAlt) || input_KeyPressed(input_Key_RAlt); }

void input_source_PressKey(input_Key key) {
	__input_KeyMask[key / 64] |= (1ull << key % 64);
	if (input_EventQueue) {
		input_Event e = {
			.Type = input_EventType_KeyPressed,
			.Key  = {
				 .Key     = key,
				 .Control = flagControl(),
				 .Shift   = flagShift(),
				 .Alt     = flagAlt(),
            },
		};
		INTERRUPT_DISABLE;
		queue_Push(input_EventQueue, &e, sizeof(e));
		INTERRUPT_RESTORE;
	}
	notify(NULL);
}
void input_source_ReleaseKey(input_Key key) {
	__input_KeyMask[key / 64] &= ~(1ull << key % 64);
	if (input_EventQueue) {
		input_Event e = {
			.Type = input_EventType_KeyReleased,
			.Key  = {
				 .Key     = key,
				 .Control = flagControl(),
				 .Shift   = flagShift(),
				 .Alt     = flagAlt(),
            },
		};
		INTERRUPT_DISABLE;
		queue_Push(input_EventQueue, &e, sizeof(e));
		INTERRUPT_RESTORE;
	}
	notify(NULL);
}

void input_source_PressMouse(input_MouseButton key) {
	__input_MouseMask[key / 64] |= (1ull << key & 64);
	if (input_EventQueue) {
		input_Event e = {
			.Type        = input_EventType_MouseButtonPressed,
			.MouseButton = {
				.Button = key,
				.X      = __input_CursorX,
				.Y      = __input_CursorY,
			},
		};
		INTERRUPT_DISABLE;
		queue_Push(input_EventQueue, &e, sizeof(e));
		INTERRUPT_RESTORE;
	}
	notify(NULL);
}
void input_source_ReleaseMouse(input_MouseButton key) {
	__input_MouseMask[key / 64] &= ~(1ull << key & 64);
	if (input_EventQueue) {
		input_Event e = {
			.Type        = input_EventType_MouseButtonReleased,
			.MouseButton = {
				.Button = key,
				.X      = __input_CursorX,
				.Y      = __input_CursorY,
			},
		};
		INTERRUPT_DISABLE;
		queue_Push(input_EventQueue, &e, sizeof(e));
		INTERRUPT_RESTORE;
	}
	notify(NULL);
}

void input_source_MoveMouse(int x, int y) {
	__input_CursorX = intminmax(__input_CursorX + x, 0, __input_DesktopWidth);
	__input_CursorY = intminmax(__input_CursorY + y, 0, __input_DesktopHeight);
	if (input_EventQueue) {
		input_Event e = {
			.Type      = input_EventType_MouseMoved,
			.MouseMove = {
				.X = __input_CursorX,
				.Y = __input_CursorY,
			},
		};
		INTERRUPT_DISABLE;
		queue_Push(input_EventQueue, &e, sizeof(e));
		INTERRUPT_RESTORE;
	}
	notify(NULL);
}
void input_source_PositionMouse(int x, int y) {
	__input_CursorX = intminmax(x, 0, __input_DesktopWidth);
	__input_CursorY = intminmax(y, 0, __input_DesktopHeight);
	if (input_EventQueue) {
		input_Event e = {
			.Type      = input_EventType_MouseMoved,
			.MouseMove = {
				.X = __input_CursorX,
				.Y = __input_CursorY,
			},
		};
		INTERRUPT_DISABLE;
		queue_Push(input_EventQueue, &e, sizeof(e));
		INTERRUPT_RESTORE;
	}
	notify(NULL);
}

void input_source_SetDesktopSize(int x, int y) {
	__input_DesktopWidth  = x;
	__input_DesktopHeight = y;
	__input_CursorX       = intminmax(__input_CursorX, 0, __input_DesktopWidth);
	__input_CursorY       = intminmax(__input_CursorY, 0, __input_DesktopHeight);
}

const char *input_Key_GetName(input_Key key) {
#define KEY(key) \
	case key:    \
		return #key;

	switch (key) {
		KEY(input_Key_A)
		KEY(input_Key_B)
		KEY(input_Key_C)
		KEY(input_Key_D)
		KEY(input_Key_E)
		KEY(input_Key_F)
		KEY(input_Key_G)
		KEY(input_Key_H)
		KEY(input_Key_I)
		KEY(input_Key_J)
		KEY(input_Key_K)
		KEY(input_Key_L)
		KEY(input_Key_M)
		KEY(input_Key_N)
		KEY(input_Key_O)
		KEY(input_Key_P)
		KEY(input_Key_Q)
		KEY(input_Key_R)
		KEY(input_Key_S)
		KEY(input_Key_T)
		KEY(input_Key_U)
		KEY(input_Key_V)
		KEY(input_Key_W)
		KEY(input_Key_X)
		KEY(input_Key_Y)
		KEY(input_Key_Z)
		KEY(input_Key_Num0)
		KEY(input_Key_Num1)
		KEY(input_Key_Num2)
		KEY(input_Key_Num3)
		KEY(input_Key_Num4)
		KEY(input_Key_Num5)
		KEY(input_Key_Num6)
		KEY(input_Key_Num7)
		KEY(input_Key_Num8)
		KEY(input_Key_Num9)
		KEY(input_Key_Escape)
		KEY(input_Key_LControl)
		KEY(input_Key_LShift)
		KEY(input_Key_LAlt)
		KEY(input_Key_LSuper)
		KEY(input_Key_RControl)
		KEY(input_Key_RShift)
		KEY(input_Key_RAlt)
		KEY(input_Key_RSuper)
		KEY(input_Key_Menu)
		KEY(input_Key_LBracket)
		KEY(input_Key_RBracket)
		KEY(input_Key_Semicolon)
		KEY(input_Key_Comma)
		KEY(input_Key_Period)
		KEY(input_Key_Quote)
		KEY(input_Key_Slash)
		KEY(input_Key_Backslash)
		KEY(input_Key_Tilde)
		KEY(input_Key_Equal)
		KEY(input_Key_Hyphen)
		KEY(input_Key_Space)
		KEY(input_Key_Enter)
		KEY(input_Key_Backspace)
		KEY(input_Key_Tab)
		KEY(input_Key_PageUp)
		KEY(input_Key_PageDown)
		KEY(input_Key_End)
		KEY(input_Key_Home)
		KEY(input_Key_Insert)
		KEY(input_Key_Delete)
		KEY(input_Key_Add)
		KEY(input_Key_Subtract)
		KEY(input_Key_Multiply)
		KEY(input_Key_Divide)
		KEY(input_Key_Left)
		KEY(input_Key_Right)
		KEY(input_Key_Up)
		KEY(input_Key_Down)
		KEY(input_Key_Numpad0)
		KEY(input_Key_Numpad1)
		KEY(input_Key_Numpad2)
		KEY(input_Key_Numpad3)
		KEY(input_Key_Numpad4)
		KEY(input_Key_Numpad5)
		KEY(input_Key_Numpad6)
		KEY(input_Key_Numpad7)
		KEY(input_Key_Numpad8)
		KEY(input_Key_Numpad9)
		KEY(input_Key_F1)
		KEY(input_Key_F2)
		KEY(input_Key_F3)
		KEY(input_Key_F4)
		KEY(input_Key_F5)
		KEY(input_Key_F6)
		KEY(input_Key_F7)
		KEY(input_Key_F8)
		KEY(input_Key_F9)
		KEY(input_Key_F10)
		KEY(input_Key_F11)
		KEY(input_Key_F12)
		KEY(input_Key_F13)
		KEY(input_Key_F14)
		KEY(input_Key_F15)
		KEY(input_Key_Pause)
		KEY(input_Key_CapsLock)
		KEY(input_Key_NumLock)
		KEY(input_Key_ScrollLock)
		default:
			return "(Unknown)";
	}
#undef KEY
}
