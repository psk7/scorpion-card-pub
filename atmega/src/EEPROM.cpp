#include "ZxKeyboard.h"
#include "EEPROM.h"
#include "HIDClassCommon.h"

EEPROM ConfigStorage;

static const struct ConfigStorage ConfigurationStorage
STORAGE = {
        .Tag = 0xAEAE,
        .Size = sizeof(ConfigurationStorage),
        .StructRevision = 1,

        .BoardSettings = {
                .ExtLinkEnabled = true,
                .Led1Enabled = true,
                .Led2Enabled = true
        },

        .KempstonMouseSettings = {
                .AxisDivider = 2,
                .WheelDivider = 1,
                .ButtonSwap = false,
                .WheelEnabled = true,
                .WheelInverse = true
        },

        .DedicatedKeys = {
                .ResetScanCode = HID_KEYBOARD_SC_F12,
                .NMIScanCode = HID_KEYBOARD_SC_F11,
                .TurboSwitchScanCode = HID_KEYBOARD_SC_F10,
                .WaitScanCode = HID_KEYBOARD_SC_PAUSE,
        },

        .TurboSettings = {
                .TurboControlEnabled = true,
                .TurboLed = HID_KEYBOARD_LED_SCROLLLOCK,
        },

        .JoystickMapping = {
                .ZxJoystickMapping = {
                        {
                                .Left = ZXKey::Joy_Left, .Right = ZXKey::Joy_Down,
                                .Up = ZXKey::Joy_Up, .Down=ZXKey::Joy_Down,
                                .Fire = ZXKey::Joy_Fire
                        },
                        {
                                .Left = ZXKey::_1, .Right = ZXKey::_2,
                                .Up = ZXKey::_4, .Down=ZXKey::_3,
                                .Fire = ZXKey::_5
                        },
                        {
                                .Left = ZXKey::_6, .Right = ZXKey::_7,
                                .Up = ZXKey::_9, .Down=ZXKey::_8,
                                .Fire = ZXKey::_0
                        },
                },
                .Mappings = {
                        {
                                .JoystickId = 1,
                                .Bindings = {
                                        {
                                                // Kempston Joystick
                                                ZXKey::Joy_Right, ZXKey::Joy_Left, ZXKey::Joy_Down, ZXKey::Joy_Up,
                                                ZXKey::Joy_Fire, ZXKey::_1, ZXKey::_2, ZXKey::_3,
                                                ZXKey::_4, ZXKey::_5, ZXKey::_6, ZXKey::_7,
                                                ZXKey::_8, ZXKey::_9, ZXKey::_9, ZXKey::Sp,
                                        },
                                        {
                                                // Sinclair 1 joystick
                                                ZXKey::_2,        ZXKey::_1,       ZXKey::_3,       ZXKey::_4,
                                                ZXKey::_5,       ZXKey::_1, ZXKey::_2, ZXKey::_3,
                                                ZXKey::_4, ZXKey::_5, ZXKey::_6, ZXKey::_7,
                                                ZXKey::_8, ZXKey::_9, ZXKey::_9, ZXKey::Sp,
                                        },
                                        {
                                                // Sinclair 2 joystick
                                                ZXKey::_7,        ZXKey::_6,       ZXKey::_8,       ZXKey::_9,
                                                ZXKey::_0,       ZXKey::_1, ZXKey::_2, ZXKey::_3,
                                                ZXKey::_4, ZXKey::_5, ZXKey::_6, ZXKey::_7,
                                                ZXKey::_8, ZXKey::_9, ZXKey::_9, ZXKey::Sp,
                                        },
                                        {
                                                // DOOM (DR)
                                                ZXKey::L,         ZXKey::J,        ZXKey::K,        ZXKey::I,
                                                ZXKey::CS,       ZXKey::CS, ZXKey::Sp, ZXKey::CS,
                                                ZXKey::A,  ZXKey::Z,  ZXKey::A,  ZXKey::Z,
                                                ZXKey::Sp, ZXKey::Sp, ZXKey::Sp, ZXKey::Sp,
                                        },
                                }
                        }
                }
        }
};

void EEPROM::operator>>(DedicatedKeysInfo &Target) {
#ifdef WIN32
    memcpy((void *) &Target, (void *) &ConfigurationStorage.DedicatedKeys, sizeof Target);
#else
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.DedicatedKeys, sizeof Target);
#endif
}

void EEPROM::operator>>(KempstonMouseSettingsInfo &Target) {
#ifdef WIN32
    memcpy((void *) &Target, (void *) &ConfigurationStorage.KempstonMouseSettings, sizeof Target);
#else
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.KempstonMouseSettings, sizeof Target);
#endif
}

void EEPROM::operator>>(TurboSettingsInfo &Target) {
#ifdef WIN32
    memcpy((void *) &Target, (void *) &ConfigurationStorage.TurboSettings, sizeof Target);
#else
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.TurboSettings, sizeof Target);
#endif
}

void EEPROM::operator>>(JoystickMappingInfo &Target) {
#ifdef WIN32
    memcpy((void *) &Target, (void *) &ConfigurationStorage.JoystickMapping, sizeof Target);
#else
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.JoystickMapping, sizeof Target);
#endif
}

void EEPROM::operator>>(BoardSettingsInfo &Target) {
#ifdef WIN32
    memcpy((void *) &Target, (void *) &ConfigurationStorage.BoardSettings, sizeof Target);
#else
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.BoardSettings, sizeof Target);
#endif
}

void EEPROM::ReadZxJoystickMappings(uint8_t MapNum, struct JoystickMappingInfo::ZxJoystickMapping &Target) {
#ifdef WIN32
    memcpy((void *) &Target, (void *) &ConfigurationStorage.JoystickMapping.ZxJoystickMapping[MapNum], sizeof Target);
#else
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.JoystickMapping.ZxJoystickMapping[MapNum],
                      sizeof Target);
#endif
}

void EEPROM::ReadJoystickMappings(uint8_t MapNum, struct JoystickMappingInfo::Mapping::Bindings &Target){
#ifdef WIN32
    memcpy((void *) &Target, (void *) &ConfigurationStorage.JoystickMapping.Mappings[0].Bindings[MapNum], sizeof Target);
#else
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.JoystickMapping.Mappings[0].Bindings[MapNum],
                      sizeof Target);
#endif
}
