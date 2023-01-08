#include "ZxKeyboard.h"
#include "ZxLayout.h"

extern const ZxLayoutKeyRef LeftShiftLayout[256];

const ZxLayoutKeyRef LeftShiftLayout[256] STORAGE = {
        {},
        {},
        {},
        {},
        {ZXKey::A | CAPS_SHIFT},                            // HID_KEYBOARD_SC_A
        {ZXKey::B | CAPS_SHIFT},                            // HID_KEYBOARD_SC_B
        {ZXKey::C | CAPS_SHIFT},                            // HID_KEYBOARD_SC_C
        {ZXKey::D | CAPS_SHIFT},                            // HID_KEYBOARD_SC_D
        {ZXKey::E | CAPS_SHIFT},                            // HID_KEYBOARD_SC_E
        {ZXKey::F | CAPS_SHIFT},                            // HID_KEYBOARD_SC_F
        {ZXKey::G | CAPS_SHIFT},                            // HID_KEYBOARD_SC_G
        {ZXKey::H | CAPS_SHIFT},                            // HID_KEYBOARD_SC_H
        {ZXKey::I | CAPS_SHIFT},                            // HID_KEYBOARD_SC_I
        {ZXKey::J | CAPS_SHIFT},                            // HID_KEYBOARD_SC_J
        {ZXKey::K | CAPS_SHIFT},                            // HID_KEYBOARD_SC_K
        {ZXKey::L | CAPS_SHIFT},                            // HID_KEYBOARD_SC_L
        {ZXKey::M | CAPS_SHIFT},                            // HID_KEYBOARD_SC_M
        {ZXKey::N | CAPS_SHIFT},                            // HID_KEYBOARD_SC_N
        {ZXKey::O | CAPS_SHIFT},                            // HID_KEYBOARD_SC_O
        {ZXKey::P | CAPS_SHIFT},                            // HID_KEYBOARD_SC_P
        {ZXKey::Q | CAPS_SHIFT},                            // HID_KEYBOARD_SC_Q
        {ZXKey::R | CAPS_SHIFT},                            // HID_KEYBOARD_SC_R
        {ZXKey::S | CAPS_SHIFT},                            // HID_KEYBOARD_SC_S
        {ZXKey::T | CAPS_SHIFT},                            // HID_KEYBOARD_SC_T
        {ZXKey::U | CAPS_SHIFT},                            // HID_KEYBOARD_SC_U
        {ZXKey::V | CAPS_SHIFT},                            // HID_KEYBOARD_SC_V
        {ZXKey::W | CAPS_SHIFT},                            // HID_KEYBOARD_SC_W
        {ZXKey::X | CAPS_SHIFT},                            // HID_KEYBOARD_SC_X
        {ZXKey::Y | CAPS_SHIFT},                            // HID_KEYBOARD_SC_Y
        {ZXKey::Z | CAPS_SHIFT},                            // HID_KEYBOARD_SC_Z
        {ZXKey::_1 | SYM_SHIFT},                            // HID_KEYBOARD_SC_1_AND_EXCLAMATION
        {ZXKey::_2 | SYM_SHIFT},                            // HID_KEYBOARD_SC_2_AND_AT
        {ZXKey::_3 | SYM_SHIFT},                            // HID_KEYBOARD_SC_3_AND_HASHMARK
        {ZXKey::_4 | SYM_SHIFT},                            // HID_KEYBOARD_SC_4_AND_DOLLAR
        {ZXKey::_5 | SYM_SHIFT},                            // HID_KEYBOARD_SC_5_AND_PERCENTAGE
        {ZXKey::H | SYM_SHIFT},                             // HID_KEYBOARD_SC_6_AND_CARET
        {ZXKey::_6 | SYM_SHIFT},                            // HID_KEYBOARD_SC_7_AND_AMPERSAND
        {ZXKey::B | SYM_SHIFT},                             // HID_KEYBOARD_SC_8_AND_ASTERISK
        {ZXKey::_8 | SYM_SHIFT},                            // HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS
        {ZXKey::_9 | SYM_SHIFT},                            // HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS
        {},                                                 // HID_KEYBOARD_SC_ENTER
        {},                                                 // HID_KEYBOARD_SC_ESCAPE
        {},                                                 // HID_KEYBOARD_SC_BACKSPACE
        {},                                                 // HID_KEYBOARD_SC_TAB
        {},                                                 // HID_KEYBOARD_SC_SPACE
        {ZXKey::_0 | SYM_SHIFT},                            // HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE
        {ZXKey::K | SYM_SHIFT},                             // HID_KEYBOARD_SC_EQUAL_AND_PLUS
        {ZXKey::F | SYM_SHIFT},                             // HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE
        {ZXKey::G | SYM_SHIFT},                             // HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE
        {},
        {},
        {ZXKey::Z | SYM_SHIFT},                             // HID_KEYBOARD_SC_SEMICOLON_AND_COLON
        {ZXKey::P | SYM_SHIFT},                             // HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE
        {},                                                 // HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE
        {ZXKey::R | SYM_SHIFT},                             // HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN
        {ZXKey::T | SYM_SHIFT},                             // HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN
        {ZXKey::C | SYM_SHIFT},                             // HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},                                                 // HID_KEYBOARD_SC_RIGHT_ARROW
        {},                                                 // HID_KEYBOARD_SC_LEFT_ARROW
        {},                                                 // HID_KEYBOARD_SC_DOWN_ARROW
        {},                                                 // HID_KEYBOARD_SC_UP_ARROW
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},                                                 // HID_KEYBOARD_SC_KEYPAD_BACKSPACE
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},                                                 // HID_KEYBOARD_SC_LEFT_CONTROL
        {},                                                 // HID_KEYBOARD_SC_LEFT_SHIFT
        {},
        {},
        {},                                                 // HID_KEYBOARD_SC_RIGHT_CONTROL
        {},                                                 // HID_KEYBOARD_SC_RIGHT_SHIFT
        {},                                                 // HID_KEYBOARD_SC_RIGHT_ALT
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
};

