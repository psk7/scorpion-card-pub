#include "USBHost.h"
#include "SPI.h"
#include "Timers.h"
#include "usblib.h"
#include "DescriptorParser.h"
#include "Console.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedValue"
using namespace USBH;

#include "MAX3421.h"

#define WAIT_BUSY while (ReadResult() == Result::hrBUSY) co_await std::suspend_always{};
#define WAIT_HXFRDNIRQ while ((SPI::ReadUSB(HIRQ) & HXFRDNIRQ) == 0) co_await std::suspend_always{};
#define WR_HXFR(x) { SPI::WriteUSB(HXFR, (x));  WAIT_HXFRDNIRQ; SPI::WriteUSB(HIRQ, HXFRDNIRQ); }
#define WR_SETUP WR_HXFR(0x10)
#define WR_HSIN WR_HXFR(0x80)
#define WR_HSOUT WR_HXFR(0xa0)

static uint8_t usb_next_address = 1;
static RootPortInfo RootPort;
static bool indelay = false;
static task<bool> root_coro;
static DriversTable *Drivers;

std::suspend_always USBH::operator co_await(delay_res d) noexcept { indelay = true; return std::suspend_always{ }; }

Result ReadResult() {
    return (Result) (SPI::ReadUSB(HRSL) & 0xf);
}

void SaveToggle(USBTarget &Obj) {
    uint8_t hrsl = SPI::ReadUSB(HRSL);

    Obj.Flags.RcvToggle = hrsl & RCVTOGRD;
    Obj.Flags.SndToggle = hrsl & SNDTOGRD;
}

void RestoreToggle(const USBTarget &Obj) {
    uint8_t rcv = Obj.Flags.RcvToggle ? RCVTOG1 : RCVTOG0;
    uint8_t snd = Obj.Flags.SndToggle ? SNDTOG1 : SNDTOG0;

    SPI::WriteUSB(HCTL, rcv | snd);
}

void SetupTarget(const USBTarget &Obj) {
    auto v = DPPULLDN | DMPULLDN | HOST | SOFKAENAB;

    if (Obj.Flags.IsLowSpeedDevice)
        v |= LOWSPEED;

    if (!RootPort.Flags.IsLowSpeedDevice && Obj.Flags.IsLowSpeedDevice)
        v |= HUBPRE;

    SPI::WriteUSB(MODE, v);
    SPI::WriteUSB(PERADDR, Obj.UsbAddress);
}

delay_res USBH::Delay(uint8_t ms) {
    Timers_SetupUSBDelay(ms);
    return {};
}

task<Result>
ReceiveStream(void *buffer, uint8_t Size, uint8_t &ReadedBytes, bool AllowNAK = false, uint8_t EndPoint = 0) {
    ReadedBytes = 0;
    uint8_t *ptr = (uint8_t *) buffer;
    Result result;
    uint16_t nak_attempts = 10000;

    while (ReadedBytes < Size) {
        SPI::WriteUSB(HXFR, EndPoint);
        WAIT_HXFRDNIRQ
        SPI::WriteUSB(HIRQ, HXFRDNIRQ);
        result = ReadResult();

        if (result == Result::hrTOGERR)
            continue;

        if (result == Result::hrNAK && !AllowNAK && (--nak_attempts > 0))
            continue;

        if (result != Result::hrSUCCESS)
            co_return result;

        auto cnt = (unsigned int) SPI::ReadUSB(RCBVC);

        if (cnt != 0)
            SPI::ReadUSB(RCVFIFO, cnt, ptr);

        SPI::WriteUSB(HIRQ, RCVDAVIRQ);

        if (cnt == 0)
            co_return result;

        ptr += cnt;
        ReadedBytes += cnt;
    }

    co_return result;
}

StreamReader<>
USBDriver::ReceiveControlAsync(uint8_t Size, uint8_t &ReadedBytes, bool Print) {
    ReadedBytes = 0;
    Result result;
    uint16_t nak_attempts = 10000;

    bool skip = false;

    while (ReadedBytes < Size) {
        if (skip) {
            co_yield 0;
            continue;
        }

        SPI::WriteUSB(HXFR, 0);
        WAIT_HXFRDNIRQ
        SPI::WriteUSB(HIRQ, HXFRDNIRQ);
        result = ReadResult();

        if (result == Result::hrTOGERR)
            continue;

        if (result == Result::hrNAK && (--nak_attempts > 0))
            continue;

        if (result != Result::hrSUCCESS) {
            WR_HSOUT
            skip = true;
            continue;
        }

        auto cnt = (uint8_t) SPI::ReadUSB(RCBVC);

        for (uint8_t i = 0; i < cnt; ++i) {
            uint8_t byte = SPI::ReadUSB(RCVFIFO);
            CON << HEX(byte) << ' ';
            co_yield byte;
        }

        SPI::WriteUSB(HIRQ, RCVDAVIRQ);

        if (cnt == 0) {
            WR_HSOUT
            skip = true;
            continue;
        }

        ReadedBytes += cnt;
    }

    WR_HSOUT
    co_yield 0;
}

task<Result> USBControl(const USB_Request_Header_t &requestHeader) {
    WAIT_BUSY
    SPI::WriteUSBControl((uint8_t *) &requestHeader);

    SPI::WriteUSB(HXFR, 0x10);   // SETUP
    WAIT_HXFRDNIRQ
    SPI::WriteUSB(HIRQ, HXFRDNIRQ);

    co_return ReadResult();
}

#define MAKE_DESCRIPTOR_REQ(type, size) \
    USB_Request_Header_t{                                         \
        .bmRequestType = REQDIR_DEVICETOHOST,                     \
        .bRequest      = USB_REQUEST_GET_DESCRIPTOR,              \
        .wValue        = type << 8,                               \
        .wIndex        = 0,                                       \
        .wLength       = size}

task<bool> TryReadDeviceDescriptor(void *buffer, PortInfo &PortInfo, uint8_t FirstAttemptSize) {
    uint8_t readed;
    Result r;

    uint8_t attempts = 40;

    auto req = MAKE_DESCRIPTOR_REQ(USB_DEVICE_DESC_TYPE, FirstAttemptSize);

    while ((r = co_await USBControl(req)) == Result::hrJERR)
        if (--attempts == 0)
            co_return false;
        else
            co_await Delay(25);

    if (r != Result::hrSUCCESS)
        co_return false;

    SPI::WriteUSB(HCTL, RCVTOG1);

    r = co_await ReceiveStream(buffer, FirstAttemptSize, readed);

    if ((r != Result::hrSUCCESS) || (readed == 0))
        co_return false;

    WR_HSOUT

    uint8_t realDescriptorSize = ((USBD::_USB_DEVICE_DESCRIPTOR_ *) buffer)->bLength;

    co_await PortInfo.ResetPort();
    co_await Delay(200);
    SPI::WriteUSB(PERADDR, 0);

    if (readed == realDescriptorSize)
        co_return true;     // Дескриптор уже прочитан полностью

    r = co_await USBControl(MAKE_DESCRIPTOR_REQ(USB_DEVICE_DESC_TYPE, realDescriptorSize));

    if (r != Result::hrSUCCESS)
        co_return false;

    SPI::WriteUSB(HCTL, RCVTOG1);

    co_await ReceiveStream(buffer, realDescriptorSize, readed);

    WR_HSOUT

    co_return readed == realDescriptorSize;
}

task<bool> USBH::InitializeNewDevice(PortInfo &PortInfo) {
    uint16_t attempts = 4;
    uint8_t readed;

    Alloc dbuf(256);

    while (--attempts > 0) {
        auto v = DPPULLDN | DMPULLDN | HOST | SOFKAENAB;

        if (PortInfo.Flags.IsLowSpeedDevice)
            v |= LOWSPEED;

        if (!RootPort.Flags.IsLowSpeedDevice && PortInfo.Flags.IsLowSpeedDevice)
            v |= HUBPRE;

        SPI::WriteUSB(MODE, v);

        co_await PortInfo.ResetPort();
        co_await Delay(200);

        SPI::WriteUSB(PERADDR, 0);

        if (co_await TryReadDeviceDescriptor(*dbuf, PortInfo, PortInfo.Flags.IsLowSpeedDevice ? 8 : 64))
            break;

        if (co_await TryReadDeviceDescriptor(*dbuf, PortInfo, 8))
            break;

        co_await Delay(50);
    }

    if (attempts == 0)
        co_return false;

    // Установка адреса устройства
    auto deviceClass = ((USB_StdDescriptor_Device_t *) *dbuf)->bDeviceClass;
    auto addr = usb_next_address++;

    WAIT_BUSY
    auto result = co_await USBControl(
            {
                    .bmRequestType = 0,
                    .bRequest      = USB_REQUEST_SET_ADDRESS,
                    .wValue        = addr,
                    .wIndex        = 0,
                    .wLength       = 0
            });

    attempts = 1000;

    while (--attempts > 0) {
        SPI::WriteUSB(HXFR, 0x80);
        while ((SPI::ReadUSB(HIRQ) & HXFRDNIRQ) == 0)
            co_await std::suspend_always{};
        SPI::WriteUSB(HIRQ, HXFRDNIRQ);
        result = ReadResult();

        if (result == Result::hrSUCCESS)
            break;
    }

    co_await Delay(10);

    if (result != Result::hrSUCCESS)
        co_return false;

    SPI::WriteUSB(PERADDR, addr);

    PortInfo.UsbAddress = addr;

    // Устройству уже назначен адрес. Если дальнейшее конфигурирование не удастся - можно
    // оставить его в покое. Другим оно уже не помешает.

    // Чтение конфигурации
    result = co_await USBControl(MAKE_DESCRIPTOR_REQ(USB_CFG_DESC_TYPE, 9));

    SPI::WriteUSB(HCTL, RCVTOG1);
    result = co_await ReceiveStream(*dbuf, 9, readed);
    WR_HSOUT

    if (readed == 0)
        co_return true;     // Не удалось прочитать дескриптор конфигурации - устройство в дальнейшем игнорируется

    auto descSize = ((USB_StdDescriptor_Configuration_Header_t *) *dbuf)->wTotalLength;

    co_await USBControl(MAKE_DESCRIPTOR_REQ(USB_CFG_DESC_TYPE, descSize));

    SPI::WriteUSB(HCTL, RCVTOG1);
    co_await ReceiveStream(*dbuf, descSize, readed);
    WR_HSOUT

    if (readed == 0)
        co_return true;

    CON << PSTR("Configuration descriptor: len=") << DEC(descSize) << endl() << Buffer(*dbuf, descSize) << endl();

    DescriptorParser descriptor(*dbuf);

    while (descriptor.NextInterface()) {
        for (const auto &item: *Drivers) {
            if (co_await item->Analyze(PortInfo, deviceClass, descriptor))
                break;
        }
    }

    dbuf.release();

    for (const auto &item: *Drivers)
        co_await item->Probe();

    co_return true; // Не удалось настроить устройство - в дальнейшем игнорируется
}

task<bool> Coro_Init() {
    RootPort.UsbAddress = 0;

    CON << PSTR("USB Reset") << endl();

    for (const auto &item: *Drivers)
        item->Reset();

    usb_next_address = 1;
    indelay = false;

    SPI::WriteUSB(IOPINS2, 0x00);

    SPI::WriteUSB(PINCTL, FDUPSPI);    // Enable full duplex
    SPI::WriteUSB(USBCTL, CHIPRES);    // Set chip reset

    SPI::WriteUSB(IOPINS2, 0x08);

    CON << PSTR("MAX Reset...");

    co_await Delay(200);

    SPI::WriteUSB(IOPINS2, 0x00);

    SPI::WriteUSB(USBCTL, 0);  // Reset chip reset

    CON << PSTR("Done") << endl();

    co_await Delay(200);

    SPI::WriteUSB(MODE, DPPULLDN | DMPULLDN | HOST);
    SPI::WriteUSB(HIRQ, CONDETIRQ);
    SPI::WriteUSB(HCTL, SAMPLEBUS);

    uint8_t wait_attempts = 100;
    uint8_t state;

    CON << PSTR("Check device...");

    while (--wait_attempts > 0) {
        state = SPI::ReadUSB(HRSL) & (JSTATUS | KSTATUS);
        if (state == 0)
            co_await Delay(2);
        else
            break;
    }

    if (wait_attempts == 0)
        co_return false;  // Корневое устройство не обнаружено - сброс шины

    if (state & KSTATUS) {
        SPI::WriteUSB(MODE, DPPULLDN | DMPULLDN | HOST | SOFKAENAB | LOWSPEED);
        RootPort.Flags.IsLowSpeedDevice = true;
        CON << PSTR("Low speed device found") << endl();
    } else {
        SPI::WriteUSB(MODE, DPPULLDN | DMPULLDN | HOST | SOFKAENAB);
        RootPort.Flags.IsLowSpeedDevice = false;
        CON << PSTR("High speed device found") << endl();
    }

    co_await Delay(200);

    CON << PSTR("Initialize root device...") << endl();

    // Корневое устройство присутствует
    if (!co_await InitializeNewDevice(RootPort))
        co_return false;  // Не удалось подключить корневое устройство - сброс шины

    SPI::WriteUSB(HIRQ, CONDETIRQ);

    CON << PSTR("Done") << endl() << PSTR("Start polling") << endl();

    co_return true;
}

task<bool> Coro_Run() {
    co_await Delay(2);

    if ((SPI::ReadUSB(HIRQ) & CONDETIRQ) != 0)
        co_return false;  // По изменению статуса присутствия корневого устройства сброс шины

    // Опрос драйверов
    for (auto &driver: *Drivers)
        if (!co_await driver->Poll())
            co_return false;      // По запросу драйвера сброс шины

    co_return true;
}

void USBH::Init(DriversTable &Drivers) {
    ::Drivers = &Drivers;
    root_coro = Coro_Init();
}

bool USBH::IsHostTaskRunning() {
    return bool(root_coro);
}

bool USBH::Run() {
    if (indelay) {
        // В ожидании не продолжаются никакие корутины
        if (!Timers_CheckUSBDelay())
            return false;

        indelay = false;
    }

    if (!root_coro) {
        root_coro = Coro_Run();
        return false;
    }

    auto done = root_coro.done();

    if (!done) {
        root_coro.resume();
        return false;
    }

    if (!root_coro.value()) {
        // Корутина завершилась и вернула false - сброс стека
        root_coro = Coro_Init();
        return false;
    }

    root_coro.destroy();

    return true;
}

/// Запрос к контрольной конечной точке устройства. Без фазы данных.
task<bool> USBDriver::Control(USB_Request_Header_t &Request, USBTarget &Target) {
    SetupTarget(Target);

    auto r = co_await USBControl(Request);
    WR_HSIN

    co_return r == Result::hrSUCCESS;
}

/// Запрос к контрольной конечной точке устройства. С фазой чтения.
task<unsigned char *>
USBDriver::ControlRead(void *buffer, USB_Request_Header_t &Request, uint8_t Size, USBTarget &Target, bool AllowNAK) {
    uint8_t readed;

    SetupTarget(Target);

    auto result = co_await USBControl(Request);

    SPI::WriteUSB(HCTL, RCVTOG1);
    result = co_await ReceiveStream(buffer, Size, readed, AllowNAK);

    WR_HSOUT

    co_return readed != 0 ? (unsigned char *) buffer : nullptr;
}

task<void>
USBDriver::SetupControlReadAsync(USB_Request_Header_t &Request, USBTarget &Target) {
    SetupTarget(Target);

    co_await USBControl(Request);

    SPI::WriteUSB(HCTL, RCVTOG1);
}

/// Запрос к контрольной конечной точке устройства. С фазой записи.
task<void> USBDriver::ControlWrite(const USB_Request_Header_t &Command, uint8_t Size, uint8_t *data,
                                   USBTarget &Target) {
    SetupTarget(Target);
    uint8_t t[8];
    memcpy(&t[0], &Command, sizeof t);

    WAIT_BUSY
    SPI::WriteUSBControl(t);

    SPI::WriteUSB(HXFR, 0x10);      // SETUP
    co_await Delay(1);
    WAIT_HXFRDNIRQ
    SPI::WriteUSB(HIRQ, HXFRDNIRQ);

    SPI::WriteUSB(HCTL, SNDTOG1);

    SPI::WriteUSB(SNDFIFO, Size, data);
    SPI::WriteUSB(SNDBC, Size);

    WR_HXFR(0x20)       // BULK-OUT
    WR_HSIN
}

task<void> USBDriver::SetConfiguration(uint8_t ConfigurationId) {
    co_await USBControl(
            {
                    .bmRequestType = 0,
                    .bRequest      = USB_REQUEST_SET_CONFIGURATION,
                    .wValue        = ConfigurationId,
                    .wIndex        = 0,
                    .wLength       = 0
            });
    WR_HSIN
}

task<void *> USBDriver::BulkIn(void *buffer, uint8_t Size, USBTarget &Target) {
    uint8_t readed;
    SetupTarget(Target);
    RestoreToggle(Target);
    auto result = co_await ReceiveStream(buffer, Size, readed, true, 0 | Target.Endpoint);
    SaveToggle(Target);

    co_return (result == Result::hrNAK) ? nullptr : buffer;
}

/// Сброс корневого порта
task<void> RootPortInfo::ResetPort() {
    SPI::WriteUSB(HCTL, BUSRST);

    while (SPI::ReadUSB(HCTL) & BUSRST)
        co_await Delay(50);
}

#pragma clang diagnostic pop