#include "ZxKeyboard.h"
#include "EEPROM.h"
#include "HIDClassCommon.h"

EEPROM ConfigStorage;

static const struct ConfigStorage ConfigurationStorage STORAGE = {
        .Tag = 0xAEAE,
        .Size = sizeof (ConfigurationStorage),
        .StructRevision = 1,

        .BoardSettings = {
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
                .Mappings = {
                        {
                                .JoystickId = 1,
                                .Binding = {
                                        ZXKey::Joy_Right, ZXKey::Joy_Left, ZXKey::Joy_Down, ZXKey::Joy_Up,
                                        ZXKey::Joy_Fire, ZXKey::_1, ZXKey::_2, ZXKey::_3,
                                        ZXKey::_4, ZXKey::_5, ZXKey::_6, ZXKey::_7,
                                        ZXKey::_8, ZXKey::_9, ZXKey::_9, ZXKey::Sp,
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
