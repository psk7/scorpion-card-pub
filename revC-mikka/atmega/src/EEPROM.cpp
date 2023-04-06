#include "ZxKeyboard.h"
#include "EEPROM.h"
#include "HIDClassCommon.h"

#define STORAGE EEMEM

EEPROM ConfigStorage;

static const struct ConfigStorage ConfigurationStorage STORAGE = {
        .Tag = 0xAEAE,
        .Size = sizeof(ConfigurationStorage),
        .StructRevision = 1,

        .BoardSettings = {
                .ExtLinkEnabled = true,
                .Led1Enabled = true,
                .Led2Enabled = true
        },

        .UserSelectedLayouts = {
                .KeyboardLayout = 0,
                .JoystickLayout = 0,
                .padding = {}
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
                                                ZXKey::Joy_Fire,  ZXKey::Joy_Fire, ZXKey::Joy_Fire, ZXKey::Joy_Fire,
                                                ZXKey::Joy_Fire,  ZXKey::Joy_Fire, ZXKey::Joy_Fire, ZXKey::Joy_Fire,
                                                ZXKey::Joy_Fire,  ZXKey::Joy_Fire, ZXKey::Joy_Fire, ZXKey::Joy_Fire,
                                        },
                                        {
                                                // Sinclair 1 joystick
                                                ZXKey::_2, ZXKey::_1, ZXKey::_3, ZXKey::_4,
                                                ZXKey::_5, ZXKey::_5, ZXKey::_5, ZXKey::_5,
                                                ZXKey::_5, ZXKey::_5, ZXKey::_5, ZXKey::_5,
                                                ZXKey::_5, ZXKey::_5, ZXKey::_5, ZXKey::_5,
                                        },
                                        {
                                                // Sinclair 2 joystick
                                                ZXKey::_7, ZXKey::_6, ZXKey::_8, ZXKey::_9,
                                                ZXKey::_0, ZXKey::_0, ZXKey::_0, ZXKey::_0,
                                                ZXKey::_0, ZXKey::_0, ZXKey::_0, ZXKey::_0,
                                                ZXKey::_0, ZXKey::_0, ZXKey::_0, ZXKey::_0,
                                        },
                                        {
                                                // DOOM (DR)
                                                ZXKey::L,  ZXKey::J,  ZXKey::K,  ZXKey::I,
                                                ZXKey::CS, ZXKey::CS, ZXKey::Sp, ZXKey::CS,
                                                ZXKey::A,  ZXKey::Z,  ZXKey::A,  ZXKey::Z,
                                                ZXKey::Sp, ZXKey::Sp, ZXKey::Sp, ZXKey::Sp,
                                        },
                                }
                        }
                }
        }
};

static void eeprom_update(const void *src, const void *dst, size_t bytes) {
    auto sptr = (unsigned char *) src;
    auto dptr = (unsigned char *) dst;

    while (bytes-- != 0) {
        if (eeprom_read_byte(dptr) != (*sptr)) {
            while (!eeprom_is_ready());

            eeprom_write_byte(dptr, *sptr);
        }

        ++sptr;
        ++dptr;
    }
}

void EEPROM::operator>>(DedicatedKeysInfo &Target) {
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.DedicatedKeys, sizeof Target);
}

void EEPROM::operator>>(KempstonMouseSettingsInfo &Target) {
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.KempstonMouseSettings, sizeof Target);
}

void EEPROM::operator>>(TurboSettingsInfo &Target) {
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.TurboSettings, sizeof Target);
}

void EEPROM::operator>>(JoystickMappingInfo &Target) {
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.JoystickMapping, sizeof Target);
}

void EEPROM::operator>>(BoardSettingsInfo &Target) {
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.BoardSettings, sizeof Target);
}

void EEPROM::operator>>(UserSelectedLayoutInfo &Target) {
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.UserSelectedLayouts, sizeof Target);
}

void EEPROM::operator<<(UserSelectedLayoutInfo &Target) {
    eeprom_update((const unsigned char *) &Target, (const void *) &ConfigurationStorage.UserSelectedLayouts,
                  sizeof Target);
}

void EEPROM::operator<<(KempstonMouseSettingsInfo &Target) {
    eeprom_update((const unsigned char *) &Target, (const void *) &ConfigurationStorage.KempstonMouseSettings,
                  sizeof Target);
}

void EEPROM::ReadZxJoystickMappings(uint8_t MapNum, struct JoystickMappingInfo::ZxJoystickMapping &Target) {
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.JoystickMapping.ZxJoystickMapping[MapNum],
                      sizeof Target);
}

void EEPROM::ReadJoystickMappings(uint8_t MapNum, struct JoystickMappingInfo::Mapping::Bindings &Target) {
    eeprom_read_block(&Target, (const void *) &ConfigurationStorage.JoystickMapping.Mappings[0].Bindings[MapNum],
                      sizeof Target);
}

uint8_t EEPROM::GetConfigByte(uint16_t Address) {
    return eeprom_read_byte((uint8_t *) Address);
}

