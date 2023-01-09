#include "USBHost.h"
#include "SPI.h"
#include "Timers.h"
#include "avr/io.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedValue"
using namespace USBH;

#include "MAX3421.h"

#define COROUTINES_HANDLES_SIZE 10

#define WAIT_BUSY while (ReadResult() == Result::hrBUSY) co_await std::suspend_always{};
#define WAIT_HXFRDNIRQ while ((SPI::ReadUSB(HIRQ) & HXFRDNIRQ) == 0) co_await std::suspend_always{};
#define WR_HXFR(x) { SPI::WriteUSB(HXFR, (x));  WAIT_HXFRDNIRQ; SPI::WriteUSB(HIRQ, HXFRDNIRQ); }
#define WR_SETUP WR_HXFR(0x10)
#define WR_HSIN WR_HXFR(0x80)
#define WR_HSOUT WR_HXFR(0xa0)

extern unsigned char *heap_ptr;
extern unsigned char *heap_start;
unsigned char *heap_ptr = heap_start;
static std::coroutine_handle<> handles[COROUTINES_HANDLES_SIZE] = {};
static uint8_t curidx = 0;

void *usb_coro_allocator::operator new(std::size_t size) noexcept {
    auto p = heap_ptr;
    heap_ptr += size;
    return p;
}

void usb_coro_allocator::operator delete(void *ptr, std::size_t size) { heap_ptr -= size; }

void usb_coro_allocator::register_handle(std::coroutine_handle<> handle) { handles[curidx++] = handle; }

void usb_coro_allocator::unregister_handle() { handles[--curidx] = {}; }

static uint8_t buffer[USB_BUFFER_SIZE];
static uint8_t usb_next_address = 1;
static RootPortInfo RootPort;
static bool indelay = false;

static DriversTable *Drivers;

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

task<Result> ReceiveStream(uint8_t Size, uint8_t &ReadedBytes, bool AllowNAK = false, uint8_t EndPoint = 0) {
    ReadedBytes = 0;
    uint8_t *ptr = &buffer[0];
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

std::suspend_always USBH::operator co_await(delay_res d) noexcept { indelay = true;return std::suspend_always{ }; }

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

task<bool> TryReadDeviceDescriptor(PortInfo &PortInfo, uint8_t FirstAttemptSize) {
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

    r = co_await ReceiveStream(FirstAttemptSize, readed);

    if ((r != Result::hrSUCCESS) || (readed == 0))
        co_return false;

    WR_HSOUT

    uint8_t realDescriptorSize = ((_USB_DEVICE_DESCRIPTOR_ *) buffer)->bLength;

    co_await PortInfo.ResetPort();
    co_await Delay(200);
    SPI::WriteUSB(PERADDR, 0);

    if (readed == realDescriptorSize)
        co_return true;     // Дескриптор уже прочитан полностью

    r = co_await USBControl(MAKE_DESCRIPTOR_REQ(USB_DEVICE_DESC_TYPE, realDescriptorSize));

    if (r != Result::hrSUCCESS)
        co_return false;

    SPI::WriteUSB(HCTL, RCVTOG1);

    co_await ReceiveStream(realDescriptorSize, readed);

    WR_HSOUT

    co_return readed == realDescriptorSize;
}

task<bool> USBH::InitializeNewDevice(PortInfo &PortInfo) {
    uint16_t attempts = 4;
    uint8_t readed;

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

        if (co_await TryReadDeviceDescriptor(PortInfo, PortInfo.Flags.IsLowSpeedDevice ? 8 : 64))
            break;

        if (co_await TryReadDeviceDescriptor(PortInfo, 8))
            break;

        co_await Delay(50);
    }

    if (attempts == 0)
        co_return false;

    // Установка адреса устройства
    auto deviceClass = ((USB_StdDescriptor_Device_t *) buffer)->bDeviceClass;
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
    result = co_await ReceiveStream(9, readed);
    WR_HSOUT

    if (readed == 0)
        co_return true;     // Не удалось прочитать дескриптор конфигурации - устройство в дальнейшем игнорируется

    auto descSize = ((USB_StdDescriptor_Configuration_Header_t *) buffer)->wTotalLength;

    if (descSize != readed) {
        if (descSize > sizeof buffer)
            co_return true;      // Дескриптор конфигурации больше доступного буфера - устройство в дальнейшем игнорируется

        result = co_await USBControl(MAKE_DESCRIPTOR_REQ(USB_CFG_DESC_TYPE, descSize));

        SPI::WriteUSB(HCTL, RCVTOG1);
        result = co_await ReceiveStream(descSize, readed);
        WR_HSOUT
    }

    if (readed == 0)
        co_return true;     // Не удалось прочитать дескриптор конфигурации - устройство в дальнейшем игнорируется

    // Разбор дескриптора конфигурации
    uint8_t *ptr = &buffer[0];
    auto *confDescriptor = (USB_StdDescriptor_Configuration_Header_t *) ptr;

    auto ds = confDescriptor->wTotalLength;

    while ((ptr - &buffer[0]) < ds) {
        if (ptr[1] == 4) {
            for (const auto &item: *Drivers)
                if (item != nullptr)
                    if (co_await item->Analyze(PortInfo, deviceClass,
                                               confDescriptor, (USB_StdDescriptor_Interface_t *) ptr))
                        co_return true;

            // Устройство не опознано ни одним драйвером - в дальнейшем игнорируется
            co_return true;
        }

        ptr += ptr[0];
    }

    co_return true; // Не удалось настроить устройство - в дальнейшем игнорируется
}

task<void> USBHost_Coro() {
    RootPort.UsbAddress = 0;

    for (const auto &item: *Drivers)
        item->Reset();

    usb_next_address = 1;
    indelay = false;

    SPI::WriteUSB(IOPINS2, 0x00);

    SPI::WriteUSB(PINCTL, FDUPSPI);    // Enable full duplex
    SPI::WriteUSB(USBCTL, CHIPRES);    // Set chip reset

    SPI::WriteUSB(IOPINS2, 0x08);

    co_await Delay(200);

    SPI::WriteUSB(IOPINS2, 0x00);

    SPI::WriteUSB(USBCTL, 0);  // Reset chip reset

    co_await Delay(200);

    SPI::WriteUSB(MODE, DPPULLDN | DMPULLDN | HOST);
    SPI::WriteUSB(HIRQ, CONDETIRQ);
    SPI::WriteUSB(HCTL, SAMPLEBUS);

    uint8_t wait_attempts = 100;
    uint8_t state;

    while (--wait_attempts > 0) {
        state = SPI::ReadUSB(HRSL) & (JSTATUS | KSTATUS);
        if (state == 0)
            co_await Delay(2);
        else
            break;
    }

    if (wait_attempts == 0)
        co_return;  // Корневое устройство не обнаружено - сброс шины

    if (state & KSTATUS) {
        SPI::WriteUSB(MODE, DPPULLDN | DMPULLDN | HOST | SOFKAENAB | LOWSPEED);
        RootPort.Flags.IsLowSpeedDevice = true;
    } else {
        SPI::WriteUSB(MODE, DPPULLDN | DMPULLDN | HOST | SOFKAENAB);
        RootPort.Flags.IsLowSpeedDevice = false;
    }

    co_await Delay(200);

    // Корневое устройство присутствует
    if (!co_await InitializeNewDevice(RootPort))
        co_return;  // Не удалось подключить корневое устройство - сброс шины

    SPI::WriteUSB(HIRQ, CONDETIRQ);

    while (true) {
        if ((SPI::ReadUSB(HIRQ) & CONDETIRQ) != 0)
            co_return;  // По изменению статуса присутствия корневого устройства сброс шины

        // Опрос драйверов
        for (auto &driver: *Drivers)
            if (!co_await driver->Poll())
                co_return;      // По запросу драйвера сброс шины
    }
}

void USBH::Init() {
    memset(&handles, 0, sizeof handles);
    curidx = 0;
    heap_ptr = heap_start;
    USBHost_Coro().m_handle = {};
}

void USBH::Init(DriversTable &Drivers) {
    ::Drivers = &Drivers;
    Init();
}

void USBH::Run() {
    if (indelay) {
        // В ожидании не продолжаются никакие корутины
        if (!Timers_CheckUSBDelay())
            return;

        indelay = false;
    }

    // Продолжение самой глубоко вложенной корутины.
    // При завершении корутины верхнего уровня сброс шины.
    for (uint8_t i = curidx - 1; i >= 0; --i) {
        auto &h = handles[i];
        if (h && !h.done()) {
            h.resume();
            if (h.done() && (i == 0))
                USBH::Init();
            return;
        }
    }
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
USBDriver::ControlRead(USB_Request_Header_t &Request, uint8_t Size, USBTarget &Target, bool AllowNAK) {
    uint8_t readed;

    SetupTarget(Target);

    auto result = co_await USBControl(Request);

    SPI::WriteUSB(HCTL, RCVTOG1);
    result = co_await ReceiveStream(Size, readed, AllowNAK);

    WR_HSOUT

    co_return readed != 0 ? &buffer[0] : nullptr;
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

task<unsigned char *> USBDriver::BulkIn(int8_t Size, USBTarget &Target) {
    uint8_t readed;
    SetupTarget(Target);
    RestoreToggle(Target);
    auto result = co_await ReceiveStream(Size, readed, true, 0 | Target.Endpoint);
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