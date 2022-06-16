#include "ZxKeyboard.h"

const ZxLayoutKeyRef NormalLayout[256] STORAGE = {
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
        {ZXKey::_1},                                        // HID_KEYBOARD_SC_1_AND_EXCLAMATION
        {ZXKey::_2},                                        // HID_KEYBOARD_SC_2_AND_AT
        {ZXKey::_3},                                        // HID_KEYBOARD_SC_3_AND_HASHMARK
        {ZXKey::_4},                                        // HID_KEYBOARD_SC_4_AND_DOLLAR
        {ZXKey::_5},                                        // HID_KEYBOARD_SC_5_AND_PERCENTAGE
        {ZXKey::_6},                                        // HID_KEYBOARD_SC_6_AND_CARET
        {ZXKey::_7},                                        // HID_KEYBOARD_SC_7_AND_AMPERSAND
        {ZXKey::_8},                                        // HID_KEYBOARD_SC_8_AND_ASTERISK
        {ZXKey::_9},                                        // HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS
        {ZXKey::_0},                                        // HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS
        {ZXKey::Ent},                                       // HID_KEYBOARD_SC_ENTER
        {ZXKey::_1 | CAPS_SHIFT},                           // HID_KEYBOARD_SC_ESCAPE
        {ZXKey::_0 | CAPS_SHIFT},                           // HID_KEYBOARD_SC_BACKSPACE
        {ZXKey::_2},                                        // HID_KEYBOARD_SC_TAB
        {ZXKey::Sp},                                        // HID_KEYBOARD_SC_SPACE
        {ZXKey::J | SYM_SHIFT},                             // HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE
        {ZXKey::L | SYM_SHIFT},                             // HID_KEYBOARD_SC_EQUAL_AND_PLUS
        {},
        {},
        {},
        {},
        {ZXKey::O | SYM_SHIFT},                             // HID_KEYBOARD_SC_SEMICOLON_AND_COLON
        {ZXKey::_7 | SYM_SHIFT},                            // HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE
        {},
        {ZXKey::N | SYM_SHIFT},                             // HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN
        {ZXKey::M | SYM_SHIFT},                             // HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN
        {ZXKey::V | SYM_SHIFT},                             // HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
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
        {ZXKey::SS},                                        // HID_KEYBOARD_SC_RIGHT_SHIFT
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

