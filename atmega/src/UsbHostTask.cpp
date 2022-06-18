#include "UsbHostTask.h"
#include "USBHost.h"
#include "HIDDriver.h"
#include "BootProtocolMouse.h"
#include "BootProtocolKeyboard.h"
#include "HIDJoystick.h"
#include "HUBDriver.h"
#include "Link.h"
#include "ZxKeyboard.h"
#include "EEPROM.h"

HIDDriver hid;
JoystickMiniport jport;
MouseDriver mouseDriver;
KeyboardDriver keyboardDriver;
HUBDriver hubDriver;

DriversTable drivers = {&mouseDriver, &keyboardDriver, &hid, &hubDriver};

void UsbHostTask_Init() {
    WriteSync(0xA0);
    WriteSync(0xE0);

    hid.RegisterPort(jport);

    ZxKeyboard_Init();

    USBHost_Init(drivers);
}

uint8_t led = 0;

void UsbHostTask_Run(void *debugdata, bool outdevices, bool outkbd) {
    auto ptr = (uint8_t *) debugdata;

    USBHost_FillDebug(ptr);
    USBHost_Run();

    if (keyboardDriver.UsbAddress != 0 && outkbd)
        ZxKeyboard_ParseBootProtocolKeyboardReport(keyboardDriver.data);

    auto btnled = ZxKeyboard_MapJoystickAndSend(jport.GetBits());

    if (mouseDriver.NewData) {
        btnled = true;
        KempstonMouseSettingsInfo mouseSettings{};
        ConfigStorage >> mouseSettings;

        uint16_t mx = ((uint16_t) mouseDriver.GetX()) / mouseSettings.AxisDivider;
        uint16_t my = ((uint16_t) mouseDriver.GetY()) / mouseSettings.AxisDivider;
        uint8_t mw = ((mouseDriver.GetWheel() / mouseSettings.WheelDivider) << 4) & 0xf0;

        uint8_t button = (~mouseDriver.GetButtons()) & 0x7;

        WriteExt(((mw | button) ^ 0xae) | (0 << 8));    // FADF
        WriteExt((mx ^ 0xea) | (1 << 8));               // FBDF
        WriteExt((my ^ 0x77) | (2 << 8));               // FFDF

        mouseDriver.NewData = false;
    }

    WriteSync(btnled ? 4 : 0);

    WriteUSB(21, (led++ >> 3) & 0x8);

    if (outdevices) {
        *(ptr++) = jport.GetBits();
        *(ptr++) = mouseDriver.UsbAddress;
        *(ptr++) = mouseDriver.NewData;
        *(ptr++) = keyboardDriver.UsbAddress;

        for (const auto &b: mouseDriver.data)
            *(ptr++) = b;

        for (const auto &b: keyboardDriver.data)
            *(ptr++) = b;
    }

    // Check hard reset and NMI
    if (keyboardDriver.UsbAddress != 0) {
        DedicatedKeysInfo keys{};
        ConfigStorage >> keys;

        bool IsHardReset = false;
        bool IsNMI = false;
        bool IsTurboSwitch = false;

        for (uint8_t i = 2; i < 8; ++i)
            for (auto &item: keyboardDriver.data) {
                IsHardReset |= (item == keys.ResetScanCode);
                IsNMI |= (item == keys.NMIScanCode);
                IsTurboSwitch |= (item == keys.TurboSwitchScanCode);
            }

#ifndef WIN32
        if (IsHardReset)
            DDRD |= _BV(DDD3);          // Sink
        else
            DDRD &= ~_BV(DDD3);         // Z state

        if (IsNMI)
            DDRD |= _BV(DDD7);          // Sink
        else
            DDRD &= ~_BV(DDD7);         // Z state

        if (IsTurboSwitch)
            DDRD |= _BV(DDD6);          // Sink
        else
            DDRD &= ~_BV(DDD6);         // Z state
#endif
    }
}

unsigned char UsbHostTask_GetCurrentState() {
    return USBHost_GetCurrentState();
}
