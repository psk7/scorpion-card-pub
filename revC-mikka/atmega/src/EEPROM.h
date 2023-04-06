#ifndef SC_MULTICARD_EEPROM_H
#define SC_MULTICARD_EEPROM_H

#include <stdint.h>
#include <avr/eeprom.h>

struct DedicatedKeysInfo {
    uint8_t ResetScanCode;
    uint8_t NMIScanCode;
    uint8_t TurboSwitchScanCode;
    uint8_t WaitScanCode;
};

struct KempstonMouseSettingsInfo {
    uint8_t AxisDivider;
    uint8_t WheelDivider;
    bool ButtonSwap;
    bool WheelEnabled;
    bool WheelInverse;
};

struct TurboSettingsInfo {
    bool TurboControlEnabled;
    uint8_t TurboLed;
};

struct JoystickMappingInfo {
    struct ZxJoystickMapping {
        uint8_t Left, Right, Up, Down, Fire;
    } ZxJoystickMapping[4];
    struct Mapping {
        uint16_t JoystickId;

        struct Bindings {
            uint8_t Binding[16];
        } Bindings[4];
    } Mappings[4];
};

struct BoardSettingsInfo {
    bool ExtLinkEnabled;
    bool Led1Enabled;
    bool Led2Enabled;
};

struct UserSelectedLayoutInfo {
    uint8_t KeyboardLayout;
    uint8_t JoystickLayout;
    uint8_t padding[14];
};

struct ConfigStorage {
    uint16_t Tag;
    uint16_t Size;
    uint8_t StructRevision;

    BoardSettingsInfo BoardSettings;

    UserSelectedLayoutInfo UserSelectedLayouts;

    KempstonMouseSettingsInfo KempstonMouseSettings;

    DedicatedKeysInfo DedicatedKeys;

    TurboSettingsInfo TurboSettings;

    JoystickMappingInfo JoystickMapping;
};

class EEPROM {
public:
    void operator>>(DedicatedKeysInfo &Target);

    void operator>>(KempstonMouseSettingsInfo &Target);

    void operator>>(TurboSettingsInfo &Target);

    void operator>>(JoystickMappingInfo &Target);

    void operator>>(BoardSettingsInfo &Target);

    void operator>>(UserSelectedLayoutInfo &Target);
    void operator<<(UserSelectedLayoutInfo &Target);
    void operator<<(KempstonMouseSettingsInfo &Target);

    void ReadZxJoystickMappings(uint8_t MapNum, struct JoystickMappingInfo::ZxJoystickMapping &Target);

    void ReadJoystickMappings(uint8_t MapNum, struct JoystickMappingInfo::Mapping::Bindings &Target);

    uint8_t GetConfigByte(uint16_t Address);
};

extern EEPROM ConfigStorage;

#endif //SC_MULTICARD_EEPROM_H
