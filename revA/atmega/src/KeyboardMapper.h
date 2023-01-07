#ifndef CH_KEYBOARDMAPPER_H
#define CH_KEYBOARDMAPPER_H

#include "KeysList.h"
#include "ZxKeyboard.h"
#include "EEPROM.h"

class KeyboardMapper {
    struct JoystickMappingInfo::ZxJoystickMapping zjm{};
    struct JoystickMappingInfo::Mapping::Bindings binds{};
    KeysList &ZxKeys;

public:
    KeyboardMapper() = delete;
    explicit KeyboardMapper(uint8_t JoystickMap, KeysList &ZxKeys);

    void MapKeyboard(bool IsLeftShiftPressed, const KeysList &ScanCodes);

    void MapJoystick(uint16_t JoystickBits);

    void MapZxJoystick(uint8_t ZxJoystickBits);
};

#endif //CH_KEYBOARDMAPPER_H
