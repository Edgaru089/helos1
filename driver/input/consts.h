#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


// Key codes of the IBM PC (American) keyboard
typedef enum {
	input_Key_Unknown = -1, // Unhandled key
	input_Key_A       = 0,  // The A key
	input_Key_B,            // The B key
	input_Key_C,            // The C key
	input_Key_D,            // The D key
	input_Key_E,            // The E key
	input_Key_F,            // The F key
	input_Key_G,            // The G key
	input_Key_H,            // The H key
	input_Key_I,            // The I key
	input_Key_J,            // The J key
	input_Key_K,            // The K key
	input_Key_L,            // The L key
	input_Key_M,            // The M key
	input_Key_N,            // The N key
	input_Key_O,            // The O key
	input_Key_P,            // The P key
	input_Key_Q,            // The Q key
	input_Key_R,            // The R key
	input_Key_S,            // The S key
	input_Key_T,            // The T key
	input_Key_U,            // The U key
	input_Key_V,            // The V key
	input_Key_W,            // The W key
	input_Key_X,            // The X key
	input_Key_Y,            // The Y key
	input_Key_Z,            // The Z key
	input_Key_Num0,         // The 0 key
	input_Key_Num1,         // The 1 key
	input_Key_Num2,         // The 2 key
	input_Key_Num3,         // The 3 key
	input_Key_Num4,         // The 4 key
	input_Key_Num5,         // The 5 key
	input_Key_Num6,         // The 6 key
	input_Key_Num7,         // The 7 key
	input_Key_Num8,         // The 8 key
	input_Key_Num9,         // The 9 key
	input_Key_Escape,       // The Escape key
	input_Key_LControl,     // The left Control key
	input_Key_LShift,       // The left Shift key
	input_Key_LAlt,         // The left Alt key
	input_Key_LSuper,       // The left Super (Windows) key
	input_Key_RControl,     // The right Control key
	input_Key_RShift,       // The right Shift key
	input_Key_RAlt,         // The right Alt key
	input_Key_RSuper,       // The right Super (Windows) key
	input_Key_Menu,         // The Menu key
	input_Key_LBracket,     // The [ key
	input_Key_RBracket,     // The ] key
	input_Key_Semicolon,    // The ; key
	input_Key_Comma,        // The , key
	input_Key_Period,       // The . key
	input_Key_Quote,        // The ' key
	input_Key_Slash,        // The / key
	input_Key_Backslash,    // The \ key
	input_Key_Tilde,        // The ~ key
	input_Key_Equal,        // The = key
	input_Key_Hyphen,       // The - key (hyphen)
	input_Key_Space,        // The Space key
	input_Key_Enter,        // The Enter/Return keys
	input_Key_Backspace,    // The Backspace key
	input_Key_Tab,          // The Tabulation key
	input_Key_PageUp,       // The Page up key
	input_Key_PageDown,     // The Page down key
	input_Key_End,          // The End key
	input_Key_Home,         // The Home key
	input_Key_Insert,       // The Insert key
	input_Key_Delete,       // The Delete key
	input_Key_Add,          // The + key
	input_Key_Subtract,     // The - key (minus, usually from numpad)
	input_Key_Multiply,     // The * key
	input_Key_Divide,       // The / key
	input_Key_Left,         // Left arrow
	input_Key_Right,        // Right arrow
	input_Key_Up,           // Up arrow
	input_Key_Down,         // Down arrow
	input_Key_Numpad0,      // The numpad 0 key
	input_Key_Numpad1,      // The numpad 1 key
	input_Key_Numpad2,      // The numpad 2 key
	input_Key_Numpad3,      // The numpad 3 key
	input_Key_Numpad4,      // The numpad 4 key
	input_Key_Numpad5,      // The numpad 5 key
	input_Key_Numpad6,      // The numpad 6 key
	input_Key_Numpad7,      // The numpad 7 key
	input_Key_Numpad8,      // The numpad 8 key
	input_Key_Numpad9,      // The numpad 9 key
	input_Key_F1,           // The F1 key
	input_Key_F2,           // The F2 key
	input_Key_F3,           // The F3 key
	input_Key_F4,           // The F4 key
	input_Key_F5,           // The F5 key
	input_Key_F6,           // The F6 key
	input_Key_F7,           // The F7 key
	input_Key_F8,           // The F8 key
	input_Key_F9,           // The F9 key
	input_Key_F10,          // The F10 key
	input_Key_F11,          // The F11 key
	input_Key_F12,          // The F12 key
	input_Key_F13,          // The F13 key
	input_Key_F14,          // The F14 key
	input_Key_F15,          // The F15 key
	input_Key_Pause,        // The Pause key

	input_Key_Count, // Keep last - the total number of keyboard keys
} input_Key;


typedef enum {
	input_MouseButton_Left,   // Left mouse button
	input_MouseButton_Right,  // Right mouse button
	input_MouseButton_Middle, // Middle (scrollwheel) button
	input_MouseButton_4,      // 4th mouse button
	input_MouseButton_5,      // 5th mouse button

	input_MouseButton_Count // Keep last - the number of mouse buttons
} input_MouseButton;


typedef enum {
	input_EventType_MouseMoved,          // Mouse has been moved (data event.MouseMove)
	input_EventType_MouseButtonPressed,  // Mouse button has been pressed (data event.MouseButton)
	input_EventType_MouseButtonReleased, // Mouse button has been released (data event.MouseButton)
	input_EventType_KeyPressed,          // Keyboard key has been pressed (data event.Key)
	input_EventType_KeyReleased,         // Keyboard key has been released (data event.Key)
	input_EventType_Count                // Number of the event types
} input_EventType;


typedef struct {
	input_EventType Type;

	union {
		struct {
			int X, Y; // New position of the mouse
		} MouseMove;

		struct {
			input_MouseButton Button; // Button
			int               X, Y;   // Position of the mouse
		} MouseButton;

		struct {
			input_Key Key;                        // Key code
			bool      Control, Shift, Alt, Super; // Is the modifier keys pressed?
		} Key;
	};
} input_Event;


#ifdef __cplusplus
}
#endif
