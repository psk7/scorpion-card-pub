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

static struct KbdFlags {
    bool WasCS: 1;
    bool WasSS: 1;
    bool IsLeftShiftPressed: 1;
} Flags;

KeysList Keys;

struct ZxKeyMap {
    uint8_t val[6];
};

#ifdef WIN32
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

void UpdateZxKeys(const KeysList &ZxKeys) {
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

    if (new_ss && !Flags.WasSS) // SS pressed now. Pre-send it to avoid race in ZX keyboard polling
        presend |= KBD_SHIFTS_SS_MASK;

    if (new_cs && !Flags.WasCS) // CS pressed now. Pre-send it to avoid race in ZX keyboard polling
        presend |= KBD_SHIFTS_CS_MASK;

    if (presend != 0)
        WriteZxKeyboard(presend | (5 << 10));

    // Send whole bitmap to Zx Keyboard hub
    WriteZxKeyboard(row0 | (0 << 10));
    WriteZxKeyboard(row1 | (1 << 10));
    WriteZxKeyboard(row2 | (2 << 10));
    WriteZxKeyboard(row3 | (3 << 10));

    WriteZxKeyboard(kempston_joystick | (4 << 10));

    Flags.WasSS = new_ss;
    Flags.WasCS = new_cs;

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

void ZxKeyboard_MapJoystickAndSend(uint16_t JoystickBits) {
    KeysList ZxKeys;

    MapKeyboard(Flags.IsLeftShiftPressed, Keys, JoystickBits, ZxKeys);

    UpdateZxKeys(ZxKeys);
}

void ZxKeyboard_Init() {
    Flags.WasCS = false;
    Flags.WasSS = false;
    Flags.IsLeftShiftPressed = false;

    WriteZxKeyboard(0 | (0 << 10));
    WriteZxKeyboard(0 | (1 << 10));
    WriteZxKeyboard(0 | (2 << 10));
    WriteZxKeyboard(0 | (3 << 10));
    WriteZxKeyboard(0 | (4 << 10));
}

void ZxKeyboard_ProcessKeyPress(uint8_t ScanCode) {
    if (ScanCode == HID_KEYBOARD_SC_LEFT_SHIFT)
        Flags.IsLeftShiftPressed = true;

    Keys << ScanCode;
}

void ZxKeyboard_ProcessKeyRelease(uint8_t ScanCode) {
    if (ScanCode == HID_KEYBOARD_SC_LEFT_SHIFT)
        Flags.IsLeftShiftPressed = false;

    Keys -= ScanCode;
}

void ZxKeyboard_ParseBootProtocolKeyboardReport(uint8_t *Data) {
    KeysList k;

    auto mod = Data[0];

    if (mod & HID_KEYBOARD_MODIFIER_LEFTSHIFT)
        k << HID_KEYBOARD_SC_LEFT_SHIFT;

    if (mod & HID_KEYBOARD_MODIFIER_RIGHTSHIFT)
        k << HID_KEYBOARD_SC_RIGHT_SHIFT;

    if (mod & HID_KEYBOARD_MODIFIER_LEFTCTRL)
        k << HID_KEYBOARD_SC_LEFT_CONTROL;

    if (mod & HID_KEYBOARD_MODIFIER_RIGHTCTRL)
        k << HID_KEYBOARD_SC_RIGHT_CONTROL;

    if (mod & HID_KEYBOARD_MODIFIER_LEFTALT)
        k << HID_KEYBOARD_SC_LEFT_ALT;

    if (mod & HID_KEYBOARD_MODIFIER_RIGHTALT)
        k << HID_KEYBOARD_SC_RIGHT_ALT;

    for (uint8_t i = 1; i < 5; ++i)
        if (Data[i] != 0)
            k << Data[i];

    auto pressed = k;
    auto released = Keys;

    pressed -= Keys;
    released -= k;

    for (const auto &key: pressed)
        if (key != 0)
            ZxKeyboard_ProcessKeyPress(key);

    for (const auto &key: released)
        if (key != 0)
            ZxKeyboard_ProcessKeyRelease(key);
}