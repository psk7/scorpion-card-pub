#include "ZxKeyboard.h"
#include "ZxLayout.h"

extern const ZxLayoutKeyRef AlasmRusNormalLayout[256];

const ZxLayoutKeyRef AlasmRusNormalLayout[256] STORAGE = {
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
        {},                                                 // HID_KEYBOARD_SC_1_AND_EXCLAMATION
        {},                                                 // HID_KEYBOARD_SC_2_AND_AT
        {},                                                 // HID_KEYBOARD_SC_3_AND_HASHMARK
        {},                                                 // HID_KEYBOARD_SC_4_AND_DOLLAR
        {},                                                 // HID_KEYBOARD_SC_5_AND_PERCENTAGE
        {},                                                 // HID_KEYBOARD_SC_6_AND_CARET
        {},                                                 // HID_KEYBOARD_SC_7_AND_AMPERSAND
        {},                                                 // HID_KEYBOARD_SC_8_AND_ASTERISK
        {},                                                 // HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS
        {},                                                 // HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS
        {},                                                 // HID_KEYBOARD_SC_ENTER
        {ZXKey::CS | SYM_SHIFT},                            // HID_KEYBOARD_SC_ESCAPE
        {},                                                 // HID_KEYBOARD_SC_BACKSPACE
        {ZXKey::_1 | CAPS_SHIFT},                           // HID_KEYBOARD_SC_TAB
        {},                                                 // HID_KEYBOARD_SC_SPACE
        {},                                                 // HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE
        {},                                                 // HID_KEYBOARD_SC_EQUAL_AND_PLUS
        {ZXKey::Y | SYM_SHIFT},                             // HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE
        {ZXKey::_0 | SYM_SHIFT},                            // HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE
        {ZXKey::D | SYM_SHIFT},                             // HID_KEYBOARD_SC_BACKSLASH_AND_PIPE
        {},
        {ZXKey::H | SYM_SHIFT},                             // HID_KEYBOARD_SC_SEMICOLON_AND_COLON
        {ZXKey::D | SYM_SHIFT},                             // HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE
        {},                                                 // HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE
        {ZXKey::U | SYM_SHIFT},                             // HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN
        {ZXKey::_2 | SYM_SHIFT},                            // HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN
        {ZXKey::M | SYM_SHIFT},                             // HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK
        {},
        {},
        {},
        {ZXKey::I | SYM_SHIFT},                             // HID_KEYBOARD_SC_F3
        {},
        {},
        {},
        {},
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
        {ZXKey::_9 | CAPS_SHIFT},                           // HID_KEYBOARD_SC_DELETE
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

