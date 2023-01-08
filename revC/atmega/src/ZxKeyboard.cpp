#include "ZxKeyboard.h"
#include "HIDClassCommon.h"
#include "SPI.h"
#include "KeysList.h"
#include "KeyboardMapper.h"
#include "Joystick.h"

#define KBD_SHIFTS_SS_MASK 2
#define KBD_SHIFTS_CS_MASK 1

#define GET_ZX_KEY(x, y) (((x).val[((y)-1) >> 3] & (1 << (((y)-1) & 7))) != 0)
#define SET_ZX_KEY(x, y) x.val[((y)-1) >> 3] |= (1 << (((y)-1) & 7))
#define RESET_ZX_KEY(x, y) x.val[((y)-1) >> 3] &= ~(1 << (((y)-1) & 7))

void ZxKeyboard::UpdateZxKeys(const KeysList &ZxKeys) {
    struct ZxKeyMap {
        uint8_t val[6];
    };

    struct ZxKeyMap map{};

    memset(&map, 0, sizeof map);

    uint8_t &kempston_joystick = map.val[5];

    for (const auto &key: ZxKeys)
        if (key != 0)
            SET_ZX_KEY(map, key);

    bool new_ss = GET_ZX_KEY(map, ZXKey::SS);
    bool new_cs = GET_ZX_KEY(map, ZXKey::CS);

    uint16_t row0 = ((map.val[1] << 8) | (map.val[0])) & 0x3ff;
    uint16_t row1 = ((map.val[2] << 6) | (map.val[1] >> 2)) & 0x3ff;
    uint16_t row2 = ((map.val[3] << 4) | (map.val[2] >> 4)) & 0x3ff;
    uint16_t row3 = ((map.val[4] << 2) | (map.val[3] >> 6)) & 0x3ff;

    uint8_t presend = 0;

    if (new_ss && !WasSS) // SS pressed now. Pre-send it to avoid race in ZX keyboard polling
        presend |= KBD_SHIFTS_SS_MASK;

    if (new_cs && !WasCS) // CS pressed now. Pre-send it to avoid race in ZX keyboard polling
        presend |= KBD_SHIFTS_CS_MASK;

    if (presend != 0)
        SPI::WriteZxKeyboard(presend | (5 << 10));

    // Send whole bitmap to Zx Keyboard hub
    SPI::WriteZxKeyboard(row0 | (0 << 10));
    SPI::WriteZxKeyboard(row1 | (1 << 10));
    SPI::WriteZxKeyboard(row2 | (2 << 10));
    SPI::WriteZxKeyboard(row3 | (3 << 10));

    if (kempston_joystick & JOYSTICK_LEFT)
        PORTB |= _BV(PORTB2);
    else
        PORTB &= ~_BV(PORTB2);

    if (kempston_joystick & JOYSTICK_RIGHT)
        PORTB |= _BV(PORTB1);
    else
        PORTB &= ~_BV(PORTB1);

    if (kempston_joystick & JOYSTICK_UP)
        PORTD |= _BV(PORTD4);
    else
        PORTD &= ~_BV(PORTD4);

    if (kempston_joystick & JOYSTICK_DOWN)
        PORTD |= _BV(PORTD2);
    else
        PORTD &= ~_BV(PORTD2);

    // KJ Fire on ANY gamepad key
    if (kempston_joystick & JOYSTICK_FIRE)
        PORTD |= _BV(PORTD3);
    else
        PORTD &= ~_BV(PORTD3);

    WasSS = new_ss;
    WasCS = new_cs;
}

bool ZxKeyboard::MapJoystickAndSend(uint16_t JoystickBits, uint8_t ZxJoystickBits) {
    KeysList ZxKeys;

    KeyboardMapper mapper(JoyMapNum, ZxKeys, zxLayoutReference);

    mapper.MapKeyboard(IsLeftShiftPressed, RusLayout, Keys);
    mapper.MapJoystick(JoystickBits);
    mapper.MapZxJoystick(ZxJoystickBits);

    UpdateZxKeys(ZxKeys);

    bool r = false;

    for (const auto &item: ZxKeys)
        r |= (item != 0);

    return r;
}

void ZxKeyboard::Init() {
    UserSelectedLayoutInfo layoutInfo{};
    ConfigStorage >> layoutInfo;

    WasCS = false;
    WasSS = false;
    IsLeftShiftPressed = false;
    IsLeftCtrlPressed = false;
    IsLeftAltPressed = false;
    IsRightShiftPressed = false;
    IsRightCtrlPressed = false;
    IsRightAltPressed = false;
    IsWinPressed = false;
    JoyMapNum = layoutInfo.JoystickLayout;
    CapsLock = false;
    NumLock = false;

    SPI::WriteZxKeyboard(0 | (0 << 10));
    SPI::WriteZxKeyboard(0 | (1 << 10));
    SPI::WriteZxKeyboard(0 | (2 << 10));
    SPI::WriteZxKeyboard(0 | (3 << 10));

    RereadLayout(layoutInfo.KeyboardLayout);
}

void ZxKeyboard::RereadLayout(uint8_t LayoutNum) {
    if (LayoutNum > (sizeof LayoutReferences / sizeof LayoutReferences[0]))
        LayoutNum = 0;

    auto ref = &LayoutReferences[LayoutNum];
    auto iptr = (unsigned char *) ref;
    auto optr = (unsigned char *) &zxLayoutReference;
    for (uint8_t i = 0; i < sizeof zxLayoutReference; ++i)
        *(optr++) = pgm_read_byte(iptr++);
}

void ZxKeyboard::KeyPress(uint8_t ScanCode) {
    IsLeftShiftPressed |= (ScanCode == HID_KEYBOARD_SC_LEFT_SHIFT);
    IsLeftCtrlPressed |= (ScanCode == HID_KEYBOARD_SC_LEFT_CONTROL);
    IsLeftAltPressed |= (ScanCode == HID_KEYBOARD_SC_LEFT_ALT);
    IsRightShiftPressed |= (ScanCode == HID_KEYBOARD_SC_RIGHT_SHIFT);
    IsRightCtrlPressed |= (ScanCode == HID_KEYBOARD_SC_RIGHT_CONTROL);
    IsRightAltPressed |= (ScanCode == HID_KEYBOARD_SC_RIGHT_ALT);
    IsWinPressed |= (ScanCode == HID_KEYBOARD_SC_LEFT_GUI);
    IsWinPressed |= (ScanCode == HID_KEYBOARD_SC_RIGHT_GUI);

    if (ScanCode == HID_KEYBOARD_SC_CAPS_LOCK)
        CapsLock = !CapsLock;

    if (ScanCode == HID_KEYBOARD_SC_NUM_LOCK)
        NumLock = !NumLock;

    if ((IsLeftAltPressed && IsLeftCtrlPressed) || (IsWinPressed)) {
        UserSelectedLayoutInfo layoutInfo{};
        ConfigStorage >> layoutInfo;

        uint8_t oldkbdlayout = layoutInfo.KeyboardLayout;
        uint8_t oldjoylayout = layoutInfo.JoystickLayout;
        bool save = true;

        switch (ScanCode) {
            case HID_KEYBOARD_SC_F1:
                layoutInfo.KeyboardLayout = 0;
                break;
            case HID_KEYBOARD_SC_F2:
                layoutInfo.KeyboardLayout = 1;
                break;
            case HID_KEYBOARD_SC_F3:
                layoutInfo.KeyboardLayout = 2;
                break;
            case HID_KEYBOARD_SC_F4:
                layoutInfo.KeyboardLayout = 3;
                break;
            case HID_KEYBOARD_SC_F5:
                layoutInfo.JoystickLayout = 0;
                break;
            case HID_KEYBOARD_SC_F6:
                layoutInfo.JoystickLayout = 1;
                break;
            case HID_KEYBOARD_SC_F7:
                layoutInfo.JoystickLayout = 2;
                break;
            case HID_KEYBOARD_SC_F8:
                layoutInfo.JoystickLayout = 3;
                break;
            default:
                save = false;
                break;
        }

        if (save) {
            JoyMapNum = layoutInfo.JoystickLayout;

            if (oldkbdlayout != layoutInfo.KeyboardLayout) {
                RereadLayout(layoutInfo.KeyboardLayout);
                ConfigStorage << layoutInfo;
            }

            if (oldjoylayout != layoutInfo.JoystickLayout) {
                ConfigStorage << layoutInfo;
            }

            return;
        }
    }

    if (IsLeftShiftPressed && IsRightShiftPressed)
        RusLayout = !RusLayout;

    Keys += ScanCode;
}

void ZxKeyboard::KeyRelease(uint8_t ScanCode) {
    IsLeftShiftPressed &= (ScanCode != HID_KEYBOARD_SC_LEFT_SHIFT);
    IsLeftCtrlPressed &= (ScanCode != HID_KEYBOARD_SC_LEFT_CONTROL);
    IsLeftAltPressed &= (ScanCode != HID_KEYBOARD_SC_LEFT_ALT);
    IsRightShiftPressed &= (ScanCode != HID_KEYBOARD_SC_RIGHT_SHIFT);
    IsRightCtrlPressed &= (ScanCode != HID_KEYBOARD_SC_RIGHT_CONTROL);
    IsRightAltPressed &= (ScanCode != HID_KEYBOARD_SC_RIGHT_ALT);
    IsWinPressed &= (ScanCode != HID_KEYBOARD_SC_LEFT_GUI);
    IsWinPressed &= (ScanCode != HID_KEYBOARD_SC_RIGHT_GUI);

    Keys -= ScanCode;
}

void ZxKeyboard::ParseBootProtocolKeyboardReport(uint8_t *Data) {
    KeysList Target;

    auto mod = Data[0];

    if (mod & HID_KEYBOARD_MODIFIER_LEFTSHIFT)
        Target << HID_KEYBOARD_SC_LEFT_SHIFT;

    if (mod & HID_KEYBOARD_MODIFIER_RIGHTSHIFT)
        Target << HID_KEYBOARD_SC_RIGHT_SHIFT;

    if (mod & HID_KEYBOARD_MODIFIER_LEFTCTRL)
        Target << HID_KEYBOARD_SC_LEFT_CONTROL;

    if (mod & HID_KEYBOARD_MODIFIER_RIGHTCTRL)
        Target << HID_KEYBOARD_SC_RIGHT_CONTROL;

    if (mod & HID_KEYBOARD_MODIFIER_LEFTALT)
        Target << HID_KEYBOARD_SC_LEFT_ALT;

    if (mod & HID_KEYBOARD_MODIFIER_RIGHTALT)
        Target << HID_KEYBOARD_SC_RIGHT_ALT;

    if (mod & HID_KEYBOARD_MODIFIER_LEFTGUI)
        Target << HID_KEYBOARD_SC_LEFT_GUI;

    if (mod & HID_KEYBOARD_MODIFIER_RIGHTGUI)
        Target << HID_KEYBOARD_SC_RIGHT_GUI;

    for (uint8_t i = 1; i < 5; ++i)
        if (Data[i] != 0)
            Target << Data[i];

    Pressed = Target;
    Released = Keys;

    Pressed -= Keys;
    Released -= Target;

    for (const auto &key: Pressed)
        if (key != 0)
            KeyPress(key);

    for (const auto &key: Released)
        if (key != 0)
            KeyRelease(key);
}

bool ZxKeyboard::IsCtrlPressed() const {
    return IsLeftCtrlPressed || IsRightCtrlPressed;
}

bool ZxKeyboard::IsAltPressed() const {
    return IsLeftAltPressed || IsRightAltPressed;
}

bool ZxKeyboard::IsKeyPressed(uint8_t ScanCode) {
    for (const auto &item: Keys)
        if (item == ScanCode)
            return true;

    return false;
}

bool ZxKeyboard::CapsLockStatus() {
    return CapsLock;
}

bool ZxKeyboard::NumLockStatus() {
    return NumLock;
}

bool ZxKeyboard::RusLayoutStatus() {
    return RusLayout;
}
