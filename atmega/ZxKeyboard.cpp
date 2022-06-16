#include "ZxKeyboard.h"
#include "HIDClassCommon.h"
#include "Link.h"

#ifdef WIN32
#include <iostream>
#endif

#define KBD_SHIFTS_SS_MASK 2
#define KBD_SHIFTS_CS_MASK 1
#define MAX_PRESSED_KEYS 8

static struct KbdFlags {
    bool WasCS: 1;
    bool WasSS: 1;
    bool IsLeftShiftPressed: 1;
    bool WasLeftShift: 1;
    bool WasRightShift: 1;
    bool WasCtrl: 1;
} Flags;

uint8_t PrevKbdData[8];
uint8_t PressedKeys[MAX_PRESSED_KEYS];

union ZxKeyMap {
    uint8_t val[5];
};

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

        default:
            std::cout << "-";
            break;
    }
}
#endif

static inline const ZxLayoutKeyRef // NOLINT(readability-const-return-type)
GetLayoutItem(const ZxLayoutKeyRef *Layout, uint8_t Item) {
#ifdef WIN32
    auto &key = Layout[Item];

    if (key.Key1 != 0)
        return key;

    return NormalLayout[Item];
#else
    auto s = pgm_read_word(&Layout[Item]);

    if (s == 0)
        s = pgm_read_word(&NormalLayout[Item]);

    return ZxLayoutKeyRef{
            .Key1 = static_cast<uint8_t>(s & 0xff),
            .Key2 = static_cast<uint8_t>((s >> 8) & 0xff)};
#endif
}

void UpdateZxKeys() {
    union ZxKeyMap map{};

    const ZxLayoutKeyRef *Layout = Flags.IsLeftShiftPressed ? &LeftShiftLayout[0] : &NormalLayout[0];

    memset(&map, 0, sizeof map);

    for (const auto &item: PressedKeys) {
        auto &key = GetLayoutItem(Layout, item);
        auto zkey = key.Key1 & 0x3f;

        if (key.Key1 & CAPS_SHIFT)
            SET_ZX_KEY(map, ZXKey::CS);

        if (key.Key1 & SYM_SHIFT)
            SET_ZX_KEY(map, ZXKey::SS);

        SET_ZX_KEY(map, zkey);
    }

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

    Flags.WasSS = new_ss;
    Flags.WasCS = new_cs;

#ifdef WIN32
    bool of = false;
    for (int i = 1; i <= 40; ++i) {
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

void ZxKeyboard_Init() {
    Flags.WasCS = false;
    Flags.WasSS = false;
    Flags.IsLeftShiftPressed = false;
    Flags.WasLeftShift = false;
    Flags.WasRightShift = false;
    Flags.WasCtrl = false;

    WriteZxKeyboard(0 | (0 << 10));
    WriteZxKeyboard(0 | (1 << 10));
    WriteZxKeyboard(0 | (2 << 10));
    WriteZxKeyboard(0 | (3 << 10));
    WriteZxKeyboard(0 | (4 << 10));
}

void ZxKeyboard_ProcessKeyPress(uint8_t ScanCode) {
    if (ScanCode == HID_KEYBOARD_SC_LEFT_SHIFT)
        Flags.IsLeftShiftPressed = true;

    for (auto &key: PressedKeys)
        if (key == ScanCode || key == 0) {
            key = ScanCode;
            break;
        }

    UpdateZxKeys();
}

void ZxKeyboard_ProcessKeyRelease(uint8_t ScanCode) {
    if (ScanCode == HID_KEYBOARD_SC_LEFT_SHIFT)
        Flags.IsLeftShiftPressed = false;

    uint8_t Temp[MAX_PRESSED_KEYS];
    memcpy(&Temp, &PressedKeys, sizeof PressedKeys);
    memset(&PressedKeys, 0, sizeof PressedKeys);

    uint8_t *ptr = &PressedKeys[0];

    for (const auto &item: Temp)
        if (item != ScanCode && item != 0)
            *(ptr++) = item;

    UpdateZxKeys();
}

void ZxKeyboard_ParseBootProtocolKeyboardReport(uint8_t *Data) {
    //auto f1 = PrevKbdData[0];
    //auto f2 = Data[0];

    auto mod = Data[0];

    auto ls = (mod & HID_KEYBOARD_MODIFIER_LEFTSHIFT) != 0;
    auto rs = (mod & HID_KEYBOARD_MODIFIER_RIGHTSHIFT) != 0;
    auto ctrl = ((mod & HID_KEYBOARD_MODIFIER_RIGHTCTRL) | (mod & HID_KEYBOARD_MODIFIER_LEFTCTRL)) != 0;

    if (ls && !Flags.WasLeftShift)
        ZxKeyboard_ProcessKeyPress(HID_KEYBOARD_SC_LEFT_SHIFT);

    if (!ls && Flags.WasLeftShift)
        ZxKeyboard_ProcessKeyRelease(HID_KEYBOARD_SC_LEFT_SHIFT);

    if (rs && !Flags.WasRightShift)
        ZxKeyboard_ProcessKeyPress(HID_KEYBOARD_SC_RIGHT_SHIFT);

    if (!rs && Flags.WasRightShift)
        ZxKeyboard_ProcessKeyRelease(HID_KEYBOARD_SC_RIGHT_SHIFT);

    if (ctrl && !Flags.WasCtrl)
        ZxKeyboard_ProcessKeyPress(HID_KEYBOARD_SC_LEFT_CONTROL);

    if (!ctrl && Flags.WasCtrl)
        ZxKeyboard_ProcessKeyRelease(HID_KEYBOARD_SC_LEFT_CONTROL);

    Flags.WasRightShift = rs;
    Flags.WasCtrl = ctrl;
    Flags.WasLeftShift = ls;

    uint8_t Temp[8];
    memcpy(Temp, Data, sizeof PrevKbdData);

    for (int i = 2; i < 8; ++i)
        for (auto &item: PrevKbdData)
            if (Temp[i] == item)
                Temp[i] = 0;

    // Temp contains just pressed scan codes now
    for (int i = 2; i < 8; ++i)
        if (Temp[i] != 0)
            ZxKeyboard_ProcessKeyPress(Temp[i]);

    memcpy(Temp, PrevKbdData, sizeof PrevKbdData);
    for (int i = 2; i < 8; ++i)
        for (uint8_t j = 0; j < 8; j++)
            if (Temp[i] == Data[j])
                Temp[i] = 0;

    // Temp contains just released scan codes now
    for (int i = 2; i < 8; ++i)
        if (Temp[i] != 0)
            ZxKeyboard_ProcessKeyRelease(Temp[i]);

    memcpy(&PrevKbdData, Data, sizeof PrevKbdData);
}