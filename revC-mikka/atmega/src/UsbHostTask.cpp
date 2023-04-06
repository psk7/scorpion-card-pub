#include "UsbHostTask.h"
#include "USBHost.h"
#include "HIDJoystickDriver.h"
#include "BootProtocolKeyboard.h"
#include "HIDMouseDriver.h"
#include "HUBDriver.h"
#include "SPI.h"
#include "ZxKeyboard.h"
#include "EEPROM.h"
#include "Timers.h"
#include "avr/io.h"
#include "MAX3421.h"
#include "Console.h"

bool PrintReports = false;

// Управление стеком кадров корутин
unsigned char *heap_ptr;
extern "C" unsigned char __heap_start; // NOLINT(bugprone-reserved-identifier)

static USBH::task<bool> root_coro;

void *USBH::usb_coro_allocator::operator new(std::size_t size) noexcept {
    auto p = heap_ptr;
    heap_ptr += size;
    memset(p, 0, size);
    return p;
}

void USBH::usb_coro_allocator::operator delete(void *ptr, std::size_t size) { heap_ptr -= size; }

namespace USBT {
    HIDJoystickDriver joystickDriver;
    HIDMouseDriver mouseDriver;
    KeyboardDriver keyboardDriver;
    HUBDriver hubDriver;

    const USBH::DriversTable drivers = {&keyboardDriver, &mouseDriver, &joystickDriver, &hubDriver};

    ZxKeyboard zxKeyboard;

    static bool indelay = false;

    std::suspend_always operator co_await(delay_res d) noexcept { indelay = true;return std::suspend_always {}; }

    static delay_res Delay(uint8_t ms) {
        Timers_SetupUSBTaskDelay(ms);
        return {};
    }
}

using namespace USBT;

void USBT::Init() {
    zxKeyboard.Init();

    heap_ptr = &__heap_start;
    USBH::Init(drivers);
}

static void SendMouseData();

static KeysList zxKeyboardImage;

static void MixJoys(bool UpdateLeds) {
    auto mixedImage = zxKeyboardImage;

    if (joystickDriver.UsbAddress != 0) {
        auto jmaps = zxKeyboard.GetJoystickMapping();
        auto bits = joystickDriver.GetBits();

        for (uint8_t i = 0; i < 16; ++i)
            if ((bits & (1 << i)) != 0)
                mixedImage << jmaps.Binding[i];
    }

    zxKeyboard.UpdateZxKeys(mixedImage);

    if (PrintReports)
        CON << (char) 13 << PSTR("Zx keyboard: ") << Buffer(&mixedImage, sizeof mixedImage) << PSTR("  ");

    if (UpdateLeds)
        SPI::WriteUSB(IOPINS2, !mixedImage.empty() ? 0x08 : 0);
}

void Pins() {
    TurboSettingsInfo tsi{};
    ConfigStorage >> tsi;

    bool TurboOn = (PINB & _BV(PINB0)) != 0;
    uint8_t leds = TurboOn && tsi.TurboControlEnabled ? tsi.TurboLed : 0;

    leds |= (zxKeyboard.CapsLockStatus() || zxKeyboard.RusLayoutStatus()) ? HID_KEYBOARD_LED_CAPSLOCK : 0;
    leds |= zxKeyboard.NumLockStatus() ? HID_KEYBOARD_LED_NUMLOCK : 0;

    keyboardDriver.SetLeds(leds);
}

USBH::task<bool> Run_Coro();

void USBT::Run() {
    MixJoys(!USBH::IsHostTaskRunning());
    Pins();

    if (USBH::IsHostTaskRunning() && !USBH::Run())
        return;

    // Обработка USB хоста не активна
    SendMouseData();

    if (root_coro) {
        // Активна обработка клавиатуры
        if (indelay) {
            if (Timers_CheckUSBTaskDelay()) {
                // Ожидание завершено. Можно продолжить.
                indelay = false;
            } else {
                // Обработка клавиатуры в ожидании. Можно инициировать опрос USB.
                USBH::Run();
                return;
            }
        }

        root_coro.resume();

        if (!root_coro.done())
            return;

        root_coro.destroy();
    }

    // Обе задачи опроса USB и клавиатуры не активны. Запуск обработки клавиатуры при необходимости.
    auto hasKeyboardData = keyboardDriver.UsbAddress != 0 && keyboardDriver.NewData;
    keyboardDriver.NewData = false;

    if (hasKeyboardData) {
        root_coro = Run_Coro();

        if (root_coro.done())
            root_coro.destroy();
    } else{
        USBH::Run();
    }
}

static void SendMouseData() {
    if (!mouseDriver.NewData)
        return;

    mouseDriver.NewData = false;

    KempstonMouseSettingsInfo mouseSettings{};
    ConfigStorage >> mouseSettings;

    auto mx = (int16_t) (mouseDriver.GetX() / mouseSettings.AxisDivider);
    auto my = (int16_t) (mouseDriver.GetY() / mouseSettings.AxisDivider);
    auto mw = (int8_t) (mouseSettings.WheelEnabled ? mouseDriver.GetWheel() : 0);

    if (mouseSettings.WheelInverse)
        mw = (int8_t) (-mw);

    mw = (int8_t) (mw / mouseSettings.WheelDivider);

    uint8_t rawbuttons = mouseDriver.GetButtons();

    if (mouseSettings.ButtonSwap) {
        uint8_t b0 = (rawbuttons & 1) << 1;
        uint8_t b1 = (rawbuttons & 2) >> 1;

        rawbuttons &= ~3;
        rawbuttons |= (b0 | b1);
    }

    uint8_t button = ((~rawbuttons) & 7) | 8;

    auto emx = (uint8_t) mx;
    auto emy = (uint8_t) my;
    uint8_t emw = (((uint8_t) mw) << 4) & 0xf0;

    SPI::WriteExt((emw | button) | (0 << 8));    // FADF
    SPI::WriteExt(emx | (1 << 8));               // FBDF
    SPI::WriteExt(emy | (2 << 8));               // FFDF
    SPI::WriteExt((emw | button) | (0 << 8));    // FADF
}

USBH::task<bool> Run_Coro() {
    zxKeyboard.ParseBootProtocolKeyboardReport(keyboardDriver.data);

    bool Ctrl = zxKeyboard.IsCtrlPressed();
    bool Alt = zxKeyboard.IsAltPressed();
    bool Zero = zxKeyboard.IsKeyPressed(HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS);
    bool Nine = zxKeyboard.IsKeyPressed(HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS);
    bool Del = zxKeyboard.IsKeyPressed(HID_KEYBOARD_SC_DELETE);
    bool LWin = zxKeyboard.IsKeyPressed(HID_KEYBOARD_SC_LEFT_GUI);

    if (Ctrl && Alt && LWin){
        static const uint8_t greet[] PROGMEM = { ZXKey::H, ZXKey::E, ZXKey::L, ZXKey::L, ZXKey::O, ZXKey::Sp,
                                         ZXKey::M, ZXKey::I, ZXKey::K, ZXKey::K, ZXKey::A, ZXKey::Sp, ZXKey::Sp,
                                         ZXKey::U, ZXKey::S, ZXKey::E, ZXKey::Sp, ZXKey::T, ZXKey::H, ZXKey::E, ZXKey::Sp,
                                         ZXKey::S, ZXKey::T,
                                         ZXKey::A, ZXKey::B, ZXKey::I, ZXKey::L, ZXKey::I, ZXKey::T,
                                         ZXKey::R, ZXKey::O, ZXKey::N, ZXKey::E, ZXKey::S, ZXKey::Sp };

        zxKeyboardImage.clear();

        for (uint8_t i = 0; i < sizeof greet; ++i) {
            uint8_t item = pgm_read_byte(&greet[i]);
            zxKeyboardImage += item;

            zxKeyboard.UpdateZxKeys(zxKeyboardImage);

            co_await USBT::Delay(75);

            zxKeyboardImage -= item;

            zxKeyboard.UpdateZxKeys(zxKeyboardImage);

            co_await USBT::Delay(75);
        }
    }

    bool SwapPressed = Ctrl && Alt && Zero;
    bool WheelInversePressed = Ctrl && Alt && Nine;

    if (SwapPressed || WheelInversePressed)
    {
        KempstonMouseSettingsInfo mouseSettings{};
        ConfigStorage >> mouseSettings;
        if (SwapPressed)
            mouseSettings.ButtonSwap = !mouseSettings.ButtonSwap;

        if (WheelInversePressed)
            mouseSettings.WheelInverse = !mouseSettings.WheelInverse;

        CON << PSTR("Mouse settings: ") << Buffer(&mouseSettings, sizeof mouseSettings) << endl();

        ConfigStorage << mouseSettings;
    }

    if (Ctrl && Alt && zxKeyboard.IsKeyPressed(HID_KEYBOARD_SC_F9))
        PrintReports = !PrintReports;

    KeysList lst;
    zxKeyboard.MapJoystickAndSend(lst);

    auto prevHasCS = zxKeyboardImage.contains(ZXKey::CS);
    auto prevHasSS = zxKeyboardImage.contains(ZXKey::SS);
    auto newHasCS = lst.contains(ZXKey::CS);
    auto newHasSS = lst.contains(ZXKey::SS);

    if (prevHasCS && !prevHasSS && newHasCS && newHasSS) {
        // Конфликт CS/SS
        zxKeyboardImage -= ZXKey::CS;

        co_await USBT::Delay(25);

        zxKeyboardImage += ZXKey::SS;
        lst -= ZXKey::CS;

        co_await USBT::Delay(25);

        zxKeyboardImage = lst;

        co_await USBT::Delay(25);
    }

    zxKeyboardImage = lst;

    DedicatedKeysInfo keys{};
    ConfigStorage >> keys;
    TurboSettingsInfo tsi{};
    ConfigStorage >> tsi;

    // Check hard reset and NMI
    bool IsHardReset = false;
    bool IsNMI = false;
    bool IsTurboSwitch = false;

    for (uint8_t i = 2; i < 8; ++i)
        for (auto &item: USBT::keyboardDriver.data) {
            IsHardReset |= (item == keys.ResetScanCode);
            IsNMI |= (item == keys.NMIScanCode);
            IsTurboSwitch |= (item == keys.TurboSwitchScanCode);
        }

    IsHardReset |= (Ctrl && Alt && Del);

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

    co_return true;
}