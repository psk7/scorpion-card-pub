#include "KeyboardMapper.h"
#include "Joystick.h"
#include "EEPROM.h"

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

void KeyboardMapper::MapKeyboard(bool IsLeftShiftPressed, const KeysList &ScanCodes) {
    const ZxLayoutKeyRef *Layout = IsLeftShiftPressed ? &LeftShiftLayout[0] : &NormalLayout[0];

    for (const auto &item: ScanCodes) {
        if (item == 0)
            continue;

        auto &key = GetLayoutItem(Layout, item);
        uint8_t zkey = key.Key1 & 0x3f;

        if (key.Key1 & CAPS_SHIFT)
            ZxKeys << ZXKey::CS;

        if (key.Key1 & SYM_SHIFT)
            ZxKeys << ZXKey::SS;

        ZxKeys << zkey;
    }
}

void KeyboardMapper::MapJoystick(uint16_t JoystickBits) {
    for (uint8_t i = 0; i < 16; ++i)
        if ((JoystickBits & (1 << i)) != 0)
            ZxKeys << binds.Binding[i];
}

void KeyboardMapper::MapZxJoystick(uint8_t ZxJoystickBits) {
    if (ZxJoystickBits & JOYSTICK_LEFT)
        ZxKeys << zjm.Left;

    if (ZxJoystickBits & JOYSTICK_RIGHT)
        ZxKeys << zjm.Right;

    if (ZxJoystickBits & JOYSTICK_UP)
        ZxKeys << zjm.Up;

    if (ZxJoystickBits & JOYSTICK_DOWN)
        ZxKeys << zjm.Down;

    if (ZxJoystickBits & JOYSTICK_FIRE)
        ZxKeys << zjm.Fire;
}

KeyboardMapper::KeyboardMapper(uint8_t JoystickMap, KeysList &ZxKeys) : ZxKeys(ZxKeys) {
    ConfigStorage.ReadZxJoystickMappings(JoystickMap, zjm);
    ConfigStorage.ReadJoystickMappings(JoystickMap, binds);
}
