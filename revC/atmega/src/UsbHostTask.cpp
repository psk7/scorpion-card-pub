#include "UsbHostTask.h"
#include "USBHost.h"
#include "HIDJoystickDriver.h"
#include "BootProtocolKeyboard.h"
#include "HUBDriver.h"
#include "SPI.h"
#include "ZxKeyboard.h"
#include "EEPROM.h"
#include "Timers.h"
#include "avr/io.h"
#include "MAX3421.h"

using namespace USBT;

HIDJoystickDriver joystickDriver;
KeyboardDriver keyboardDriver;
HUBDriver hubDriver;

const USBH::DriversTable drivers = {&keyboardDriver, &joystickDriver, &hubDriver};

ZxKeyboard zxKeyboard;

static std::coroutine_handle<> task_handle = {};
static bool indelay = false;

extern "C" unsigned char __heap_start; // NOLINT(bugprone-reserved-identifier)
unsigned char *heap_start = nullptr;

std::suspend_always USBT::operator co_await(delay_res d) noexcept { indelay = true; return std::suspend_always{ }; }

void *coro_allocator::operator new(std::size_t size) noexcept {
    heap_start = &__heap_start + size;
    return &__heap_start;
}

void coro_allocator::operator delete(void *ptr, std::size_t size) {}

void coro_allocator::register_handle(std::coroutine_handle<> handle) {
    task_handle = handle;
}

void coro_allocator::unregister_handle() {}

static delay_res Delay(uint8_t ms) {
    Timers_SetupUSBTaskDelay(ms);
    return {};
}

static struct _UsbHostFlags {
    bool SessionButtonSwap: 1;
    bool PrevSwapPressed: 1;
} Flags;

void USBT::Init() {
    Flags.SessionButtonSwap = false;
    Flags.PrevSwapPressed = false;

    zxKeyboard.Init();

    USBT::Run_Coro().m_handle = {};
    USBH::Init(drivers);
}

void USBT::Run() {
    USBH::Run();

    if (indelay) {
        // В ожидании не продолжаются никакие корутины
        if (!Timers_CheckUSBTaskDelay())
            return;

        indelay = false;
    }

    if (!task_handle.done())
        task_handle.resume();
    else
        USBT::Run_Coro().m_handle = {};
}

task<void> USBT::Run_Coro() {
    bool HasNewKeyboardData = keyboardDriver.NewData;
    keyboardDriver.NewData = false;

    PIND |= _BV(PIND1);

    if (keyboardDriver.UsbAddress != 0 && HasNewKeyboardData) {
        zxKeyboard.ParseBootProtocolKeyboardReport(keyboardDriver.data);

        bool Ctrl = zxKeyboard.IsCtrlPressed();
        bool Alt = zxKeyboard.IsAltPressed();
        bool Zero = zxKeyboard.IsKeyPressed(HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS);

        bool SwapPressed = Ctrl && Alt && Zero;

        if (SwapPressed && (SwapPressed != Flags.PrevSwapPressed))
            Flags.SessionButtonSwap = !Flags.SessionButtonSwap;

        Flags.PrevSwapPressed = SwapPressed;
    }

    auto led = zxKeyboard.MapJoystickAndSend(joystickDriver.GetBits(), 0);

    if (keyboardDriver.UsbAddress != 0)
        SPI::WriteUSB(IOPINS2, led ? 0x08 : 0);

    TurboSettingsInfo tsi{};
    ConfigStorage >> tsi;

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

        if (IsHardReset)
            DDRD |= _BV(DDD5);          // Sink
        else
            DDRD &= ~_BV(DDD5);         // Z state

        if (IsNMI)
            DDRD |= _BV(DDD7);          // Sink
        else
            DDRD &= ~_BV(DDD7);         // Z state

        if (IsTurboSwitch && tsi.TurboControlEnabled)
            DDRD |= _BV(DDD6);          // Sink
        else
            DDRD &= ~_BV(DDD6);         // Z state
    }

    bool TurboOn = (PINB & _BV(PINB0)) != 0;
    uint8_t leds = TurboOn && tsi.TurboControlEnabled ? tsi.TurboLed : 0;

    leds |= (zxKeyboard.CapsLockStatus() || zxKeyboard.RusLayoutStatus()) ? HID_KEYBOARD_LED_CAPSLOCK : 0;
    leds |= zxKeyboard.NumLockStatus() ? HID_KEYBOARD_LED_NUMLOCK : 0;

    PINC |= _BV(PINC0);

    keyboardDriver.SetLeds(leds);

    co_return;
}