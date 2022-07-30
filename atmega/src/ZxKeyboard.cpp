#include "ZxKeyboard.h"
#include "HIDClassCommon.h"
#include "Link.h"
#include "KeysList.h"
#include "KeyboardMapper.h"

#ifdef WIN32

#include <iostream>

#endif

#define KBD_SHIFTS_SS_MASK 2
#define KBD_SHIFTS_CS_MASK 1

#ifdef WIN32
struct ZxKeyMap {
    uint8_t val[6];
};

ZxKeyMap PrevZxKeys;
#endif

#define GET_ZX_KEY(x, y) (((x).val[((y)-1) >> 3] & (1 << (((y)-1) & 7))) != 0)
#define SET_ZX_KEY(x, y) x.val[((y)-1) >> 3] |= (1 << (((y)-1) & 7))
#define RESET_ZX_KEY(x, y) x.val[((y)-1) >> 3] &= ~(1 << (((y)-1) & 7))

#ifdef WIN32
#define _(x) case x: std::cout << #x; break;

void OutZxKeys(ZXKey Key) {
    switch (Key) {
        _(NoKey)
        _(CS)
        _(Z)
        _(X)
        _(C)
        _(V)
        _(A)
        _(S)
        _(D)
        _(F)
        _(G)
        _(Q)
        _(W)
        _(E)
        _(R)
        _(T)
        _(_1)
        _(_2)
        _(_3)
        _(_4)
        _(_5)
        _(_0)
        _(_9)
        _(_8)
        _(_7)
        _(_6)
        _(P)
        _(O)
        _(I)
        _(U)
        _(Y)
        _(Ent)
        _(L)
        _(K)
        _(J)
        _(H)
        _(Sp)
        _(SS)
        _(M)
        _(N)
        _(B)
        _(Joy_Left)
        _(Joy_Right)
        _(Joy_Up)
        _(Joy_Down)
        _(Joy_Fire)

        default:
            std::cout << "-";
            break;
    }
}

#endif

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
        WriteZxKeyboard(presend | (5 << 10));

    // Send whole bitmap to Zx Keyboard hub
    WriteZxKeyboard(row0 | (0 << 10));
    WriteZxKeyboard(row1 | (1 << 10));
    WriteZxKeyboard(row2 | (2 << 10));
    WriteZxKeyboard(row3 | (3 << 10));

    WriteZxKeyboard(kempston_joystick | (4 << 10));

    WasSS = new_ss;
    WasCS = new_cs;

#ifdef WIN32
    if (memcmp(&map, &PrevZxKeys, sizeof map) == 0)
        return;

    memcpy(&PrevZxKeys, &map, sizeof map);

    bool of = false;
    for (int i = 1; i <= 45; ++i) {
        auto k = (ZXKey) i;
        if (GET_ZX_KEY(map, k)) {
            OutZxKeys(k);
            std::cout << " ";
            of = true;
        }
    }

    if (!of)
        std::cout << "--NO KEYS--";

    std::cout << std::endl;
#endif
}

bool ZxKeyboard::MapJoystickAndSend(uint16_t JoystickBits, uint8_t ZxJoystickBits) {
    KeysList ZxKeys;

    KeyboardMapper mapper(JoyMapNum, ZxKeys);

    mapper.MapKeyboard(IsLeftShiftPressed, Keys);
    mapper.MapJoystick(JoystickBits);
    mapper.MapZxJoystick(ZxJoystickBits);

    UpdateZxKeys(ZxKeys);

    bool r = false;

    for (const auto &item: ZxKeys)
        r |= (item != 0);

    return r;
}

void ZxKeyboard::Init() {
    WasCS = false;
    WasSS = false;
    IsLeftShiftPressed = false;
    IsLeftCtrlPressed = false;
    IsLeftAltPressed = false;
    IsRightShiftPressed = false;
    IsRightCtrlPressed = false;
    IsRightAltPressed = false;
    IsWinPressed = false;
    JoyMapNum = 0;

    WriteZxKeyboard(0 | (0 << 10));
    WriteZxKeyboard(0 | (1 << 10));
    WriteZxKeyboard(0 | (2 << 10));
    WriteZxKeyboard(0 | (3 << 10));
    WriteZxKeyboard(0 | (4 << 10));

    WriteExt(0 | (3 << 8));
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

    if ((IsLeftAltPressed && IsLeftCtrlPressed) || (IsWinPressed))
        switch (ScanCode) {
            case HID_KEYBOARD_SC_F5:
                JoyMapNum = 0;
                return;
            case HID_KEYBOARD_SC_F6:
                JoyMapNum = 1;
                return;
            case HID_KEYBOARD_SC_F7:
                JoyMapNum = 2;
                return;
            case HID_KEYBOARD_SC_F8:
                JoyMapNum = 3;
                return;
            default:
                break;
        }

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

    ProcessKeysList(Target);
}

void ZxKeyboard::ProcessKeysList(const KeysList &List) {
    auto pressed = List;
    auto released = Keys;

    pressed -= Keys;
    released -= List;

    for (const auto &key: pressed)
        if (key != 0)
            KeyPress(key);

    for (const auto &key: released)
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
