#include "ZxKeyboard.h"
#include "ZxLayout.h"

extern const ZxLayoutKeyRef AlasmRusLeftShiftLayout[256];

const ZxLayoutKeyRef AlasmRusLeftShiftLayout[256] STORAGE = {
        {},
        {},
        {},
        {},
        {ZXKey::A},                                         // HID_KEYBOARD_SC_A
        {ZXKey::B},                                         // HID_KEYBOARD_SC_B
        {ZXKey::C},                                         // HID_KEYBOARD_SC_C
        {ZXKey::D},                                         // HID_KEYBOARD_SC_D
        {ZXKey::E},                                         // HID_KEYBOARD_SC_E
        {ZXKey::F},                                         // HID_KEYBOARD_SC_F
        {ZXKey::G},                                         // HID_KEYBOARD_SC_G
        {ZXKey::H},                                         // HID_KEYBOARD_SC_H
        {ZXKey::I},                                         // HID_KEYBOARD_SC_I
        {ZXKey::J},                                         // HID_KEYBOARD_SC_J
        {ZXKey::K},                                         // HID_KEYBOARD_SC_K
        {ZXKey::L},                                         // HID_KEYBOARD_SC_L
        {ZXKey::M},                                         // HID_KEYBOARD_SC_M
        {ZXKey::N},                                         // HID_KEYBOARD_SC_N
        {ZXKey::O},                                         // HID_KEYBOARD_SC_O
        {ZXKey::P},                                         // HID_KEYBOARD_SC_P
        {ZXKey::Q},                                         // HID_KEYBOARD_SC_Q
        {ZXKey::R},                                         // HID_KEYBOARD_SC_R
        {ZXKey::S},                                         // HID_KEYBOARD_SC_S
        {ZXKey::T},                                         // HID_KEYBOARD_SC_T
        {ZXKey::U},                                         // HID_KEYBOARD_SC_U
        {ZXKey::V},                                         // HID_KEYBOARD_SC_V
        {ZXKey::W},                                         // HID_KEYBOARD_SC_W
        {ZXKey::X},                                         // HID_KEYBOARD_SC_X
        {ZXKey::Y},                                         // HID_KEYBOARD_SC_Y
        {ZXKey::Z},                                         // HID_KEYBOARD_SC_Z
        {ZXKey::_1 | SYM_SHIFT},                            // HID_KEYBOARD_SC_1_AND_EXCLAMATION
        {ZXKey::P | SYM_SHIFT},                             // HID_KEYBOARD_SC_2_AND_AT
        {ZXKey::_3 | SYM_SHIFT},                            // HID_KEYBOARD_SC_3_AND_HASHMARK
        {ZXKey::O | SYM_SHIFT},                             // HID_KEYBOARD_SC_4_AND_DOLLAR
        {ZXKey::_5 | SYM_SHIFT},                            // HID_KEYBOARD_SC_5_AND_PERCENTAGE
        {ZXKey::Z | SYM_SHIFT},                             // HID_KEYBOARD_SC_6_AND_CARET
        {ZXKey::C | SYM_SHIFT},                             // HID_KEYBOARD_SC_7_AND_AMPERSAND
        {ZXKey::B | SYM_SHIFT},                             // HID_KEYBOARD_SC_8_AND_ASTERISK
        {ZXKey::_8 | SYM_SHIFT},                            // HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS
        {ZXKey::_9 | SYM_SHIFT},                            // HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS
        {},                                                 // HID_KEYBOARD_SC_ENTER
        {},                                                 // HID_KEYBOARD_SC_ESCAPE
        {},                                                 // HID_KEYBOARD_SC_BACKSPACE
        {ZXKey::_1 | CAPS_SHIFT},                           // HID_KEYBOARD_SC_TAB
        {},                                                 // HID_KEYBOARD_SC_SPACE
        {ZXKey::_0 | SYM_SHIFT},                            // HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE
        {ZXKey::K | SYM_SHIFT},                             // HID_KEYBOARD_SC_EQUAL_AND_PLUS
        {ZXKey::F | SYM_SHIFT},                             // HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE
        {},
        {ZXKey::V | SYM_SHIFT},                             // HID_KEYBOARD_SC_BACKSLASH_AND_PIPE
        {},
        {ZXKey::G | SYM_SHIFT},                             // HID_KEYBOARD_SC_SEMICOLON_AND_COLON
        {ZXKey::S | SYM_SHIFT},                             // HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE
        {},                                                 // HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE
        {ZXKey::A | SYM_SHIFT},                             // HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN
        {ZXKey::X | SYM_SHIFT},                             // HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN
        {ZXKey::N | SYM_SHIFT},                             // HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK
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
        {ZXKey::Q | SYM_SHIFT},                             // HID_KEYBOARD_SC_HOME
        {ZXKey::_3 | CAPS_SHIFT},                           // HID_KEYBOARD_SC_PAGE_UP
        {ZXKey::Sp | SYM_SHIFT},                            // HID_KEYBOARD_SC_DELETE
        {ZXKey::E | SYM_SHIFT},                             // HID_KEYBOARD_SC_END
        {ZXKey::_4 | CAPS_SHIFT},                           // HID_KEYBOARD_SC_PAGE_DOWN
        {ZXKey::_8 | CAPS_SHIFT},                           // HID_KEYBOARD_SC_RIGHT_ARROW
        {ZXKey::_5 | CAPS_SHIFT},                           // HID_KEYBOARD_SC_LEFT_ARROW
        {ZXKey::_6 | CAPS_SHIFT},                           // HID_KEYBOARD_SC_DOWN_ARROW
        {ZXKey::_7 | CAPS_SHIFT},                           // HID_KEYBOARD_SC_UP_ARROW
        {},
        {ZXKey::V | SYM_SHIFT},                             // HID_KEYBOARD_SC_KEYPAD_SLASH
        {ZXKey::B | SYM_SHIFT},                             // HID_KEYBOARD_SC_KEYPAD_ASTERISK
        {ZXKey::J | SYM_SHIFT},                             // HID_KEYBOARD_SC_KEYPAD_MINUS
        {ZXKey::K | SYM_SHIFT},                             // HID_KEYBOARD_SC_KEYPAD_PLUS
        {ZXKey::Ent},                                       // HID_KEYBOARD_SC_KEYPAD_ENTER
        {ZXKey::_1},                                        // HID_KEYBOARD_SC_KEYPAD_1_AND_END
        {ZXKey::_2},                                        // HID_KEYBOARD_SC_KEYPAD_2_AND_DOWN_ARROW
        {ZXKey::_3},                                        // HID_KEYBOARD_SC_KEYPAD_3_AND_PAGE_DOWN
        {ZXKey::_4},                                        // HID_KEYBOARD_SC_KEYPAD_4_AND_LEFT_ARROW
        {ZXKey::_5},                                        // HID_KEYBOARD_SC_KEYPAD_5
        {ZXKey::_6},                                        // HID_KEYBOARD_SC_KEYPAD_6_AND_RIGHT_ARROW
        {ZXKey::_7},                                        // HID_KEYBOARD_SC_KEYPAD_7_AND_HOME
        {ZXKey::_8},                                        // HID_KEYBOARD_SC_KEYPAD_8_AND_UP_ARROW
        {ZXKey::_9},                                        // HID_KEYBOARD_SC_KEYPAD_9_AND_PAGE_UP
        {ZXKey::_0},                                        // HID_KEYBOARD_SC_KEYPAD_0_AND_INSERT
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
        {ZXKey::CS},                                        // HID_KEYBOARD_SC_LEFT_CONTROL
        {},                                                 // HID_KEYBOARD_SC_LEFT_SHIFT
        {ZXKey::CS | SYM_SHIFT},                            // HID_KEYBOARD_SC_LEFT_ALT
        {},
        {ZXKey::CS},                                        // HID_KEYBOARD_SC_RIGHT_CONTROL
        {ZXKey::Sp | CAPS_SHIFT},                           // HID_KEYBOARD_SC_RIGHT_SHIFT
        {ZXKey::CS | SYM_SHIFT},                            // HID_KEYBOARD_SC_RIGHT_ALT
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

