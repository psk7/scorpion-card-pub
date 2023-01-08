#ifndef CH_KEYBOARDMAPPER_H
#define CH_KEYBOARDMAPPER_H

#include "KeysList.h"
#include "ZxKeyboard.h"
#include "EEPROM.h"
#include "ZxLayout.h"

class KeyboardMapper {
    struct JoystickMappingInfo::ZxJoystickMapping zjm{};
    struct JoystickMappingInfo::Mapping::Bindings binds{};
    KeysList &ZxKeys;
    const ZxLayoutReference &Layout;

public:
    KeyboardMapper() = delete;
    explicit KeyboardMapper(uint8_t JoystickMap, KeysList &ZxKeys, const ZxLayoutReference &Layout);

    void MapKeyboard(bool IsLeftShiftPressed, bool RusLayout, const KeysList &ScanCodes);

    void MapJoystick(uint16_t JoystickBits);

    void MapZxJoystick(uint8_t ZxJoystickBits);
};

#endif //CH_KEYBOARDMAPPER_H
