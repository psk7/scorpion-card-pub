#include "KeyboardMapper.h"
#include "Joystick.h"
#include "EEPROM.h"

static inline const ZxLayoutKeyRef // NOLINT(readability-const-return-type)
GetLayoutItem(const ZxLayoutKeyRef *Layout, const ZxLayoutKeyRef *FallbackLayout, uint8_t Item) {
    auto s = pgm_read_byte(&Layout[Item]);

    if (s == 0)
        s = pgm_read_byte(&FallbackLayout[Item]);

    return ZxLayoutKeyRef{
            .Key1 = static_cast<uint8_t>(s),
            //.Key1 = static_cast<uint8_t>(s & 0xff),
            //.Key2 = static_cast<uint8_t>((s >> 8) & 0xff)
    };
}

void KeyboardMapper::MapKeyboard(bool IsLeftShiftPressed, bool RusLayout, const KeysList &ScanCodes) {
    auto layout
            = IsLeftShiftPressed ?
              (RusLayout ? Layout.LeftShiftRusLayout : Layout.LeftShiftLayout)
                                 :
              (RusLayout ? Layout.RusLayout : Layout.BaseLayout);

    for (const auto &item: ScanCodes) {
        if (item == 0)
            continue;

        auto &key = GetLayoutItem(layout, Layout.FallbackLayout, item);
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

KeyboardMapper::KeyboardMapper(uint8_t JoystickMap, KeysList &ZxKeys, const ZxLayoutReference &Layout)
        : ZxKeys(ZxKeys), Layout(Layout) {
    ConfigStorage.ReadZxJoystickMappings(JoystickMap, zjm);
    ConfigStorage.ReadJoystickMappings(JoystickMap, binds);
}
