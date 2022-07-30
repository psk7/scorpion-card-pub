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
#include "Joystick.h"

HIDDriver hid;
JoystickMiniport jport;
MouseDriver mouseDriver;
KeyboardDriver keyboardDriver;
HUBDriver hubDriver;

DriversTable drivers = {&mouseDriver, &keyboardDriver, &hid, &hubDriver};

static struct _UsbHostFlags {
    bool SessionButtonSwap: 1;
    bool PrevSwapPressed: 1;
} Flags;

ZxKeyboard zxKeyboard;

void UsbHostTask_Init() {
    Flags.SessionButtonSwap = false;
    Flags.PrevSwapPressed = false;

    WriteSync(0xA0);
    WriteSync(0xE0);

    hid.RegisterPort(jport);

    zxKeyboard.Init();

    USBHost_Init(drivers);
}

uint8_t led = 0;

void UsbHostTask_Run(void *debugdata, bool outdevices, bool outkbd) {
    auto ptr = (uint8_t *) debugdata;

    USBHost_FillDebug(ptr);
    USBHost_Run();

    bool HasNewKeyboardData = keyboardDriver.NewData;
    keyboardDriver.NewData = false;

    if (keyboardDriver.UsbAddress != 0 && outkbd && HasNewKeyboardData) {
        zxKeyboard.ParseBootProtocolKeyboardReport(keyboardDriver.data);

        bool Ctrl = zxKeyboard.IsCtrlPressed();
        bool Alt = zxKeyboard.IsAltPressed();
        bool Zero = zxKeyboard.IsKeyPressed(HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS);

        bool SwapPressed = Ctrl && Alt && Zero;

        if (SwapPressed && (SwapPressed != Flags.PrevSwapPressed))
            Flags.SessionButtonSwap = !Flags.SessionButtonSwap;

        Flags.PrevSwapPressed = SwapPressed;
    }

#ifndef WIN32
    uint8_t zj = 0;

    if (!(PIND & _BV(PIND1)))
        zj |= JOYSTICK_LEFT;

    if (!(PINC & _BV(PINC2)))
        zj |= JOYSTICK_RIGHT;

    if (!(PIND & _BV(PIND2)))
        zj |= JOYSTICK_UP;

    if (!(PIND & _BV(PIND0)))
        zj |= JOYSTICK_DOWN;

    if (!(PIND & _BV(PIND4)))
        zj |= JOYSTICK_FIRE;
#else
    uint8_t zj = 0;
#endif

    auto btnled = zxKeyboard.MapJoystickAndSend(jport.GetBits(), zj);

    if (mouseDriver.NewData) {
        mouseDriver.NewData = false;

        btnled = true;
        KempstonMouseSettingsInfo mouseSettings{};
        ConfigStorage >> mouseSettings;

        auto mx = (int16_t)(mouseDriver.GetX() / mouseSettings.AxisDivider);
        auto my = (int16_t)(mouseDriver.GetY() / mouseSettings.AxisDivider);
        auto mw = (int8_t)(mouseSettings.WheelEnabled ? mouseDriver.GetWheel() : 0);

        if (mouseSettings.WheelInverse)
            mw = (int8_t)(-mw);

        mw = (int8_t)(mw / mouseSettings.WheelDivider);

        uint8_t rawbuttons = mouseDriver.GetButtons();

        if (mouseSettings.ButtonSwap || Flags.SessionButtonSwap) {
            uint8_t b0 = (rawbuttons & 1) << 1;
            uint8_t b1 = (rawbuttons & 2) >> 1;

            rawbuttons &= ~3;
            rawbuttons |= (b0 | b1);
        }

        uint8_t button = ((~rawbuttons) & 7) | 8;

        auto emx = (uint8_t) mx;
        auto emy = (uint8_t) my;
        uint8_t emw = (((uint8_t) mw) << 4) & 0xf0;

        WriteExt((emw | button) | (0 << 8));    // FADF
        WriteExt(emx | (1 << 8));               // FBDF
        WriteExt(emy | (2 << 8));               // FFDF
    }

    BoardSettingsInfo info{};
    ConfigStorage >> info;

    uint8_t ev = zxKeyboard.IsKeyPressed(HID_KEYBOARD_SC_PAUSE) ? 1 : 0;
    if (info.ExtLinkEnabled)
        ev |= 2;

    WriteExt(ev | (3 << 8));

    WriteSync(btnled ? 4 : 0);

    WriteUSB(21, (led++ >> 3) & 0x8);

    if (outdevices) {
        *(ptr++) = jport.GetBits();
        *(ptr++) = mouseDriver.UsbAddress;
        *(ptr++) = mouseDriver.NewData;
        *(ptr++) = keyboardDriver.UsbAddress;

        *(ptr++) = 0;
        *(ptr++) = 0;
        *(ptr++) = 0;
        *(ptr++) = 0;

        for (const auto &b: keyboardDriver.data)
            *(ptr++) = b;
    }

    // Check hard reset and NMI
    if (keyboardDriver.UsbAddress != 0 && HasNewKeyboardData) {
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
        TurboSettingsInfo tsi{};
        ConfigStorage >> tsi;

        if (IsHardReset)
            DDRD |= _BV(DDD3);          // Sink
        else
            DDRD &= ~_BV(DDD3);         // Z state

        if (IsNMI)
            DDRD |= _BV(DDD7);          // Sink
        else
            DDRD &= ~_BV(DDD7);         // Z state

        if (IsTurboSwitch && tsi.TurboControlEnabled)
            DDRD |= _BV(DDD6);          // Sink
        else
            DDRD &= ~_BV(DDD6);         // Z state

        bool TurboOn = (PIND & _BV(PIND5)) != 0;
        keyboardDriver.SetLeds(TurboOn && tsi.TurboControlEnabled ? tsi.TurboLed : 0);
#endif
    }
}

unsigned char UsbHostTask_GetCurrentState() {
    return USBHost_GetCurrentState();
}
