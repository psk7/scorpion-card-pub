#include "KeyboardMapper.h"
#include "Joystick.h"

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

void MapKeyboard(bool IsLeftShiftPressed, const KeysList &ScanCodes, uint16_t JoystickBits, KeysList &Target) {
    const ZxLayoutKeyRef *Layout = IsLeftShiftPressed ? &LeftShiftLayout[0] : &NormalLayout[0];

    for (const auto &item: ScanCodes) {
        if (item == 0)
            continue;

        auto &key = GetLayoutItem(Layout, item);
        uint8_t zkey = key.Key1 & 0x3f;

        if (key.Key1 & CAPS_SHIFT)
            Target << ZXKey::CS;

        if (key.Key1 & SYM_SHIFT)
            Target << ZXKey::SS;

        Target << zkey;
    }

    if (JoystickBits & JOYSTICK_LEFT)
        Target << ZXKey::Joy_Left;

    if (JoystickBits & JOYSTICK_RIGHT)
        Target << ZXKey::Joy_Right;

    if (JoystickBits & JOYSTICK_UP)
        Target << ZXKey::Joy_Up;

    if (JoystickBits & JOYSTICK_DOWN)
        Target << ZXKey::Joy_Down;

    if (JoystickBits & JOYSTICK_FIRE)
        Target << ZXKey::Joy_Fire;
}
