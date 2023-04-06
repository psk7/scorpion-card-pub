#include "HUBDriver.h"
#include "HIDClassCommon.h"
#include "usblib.h"
#include "SPI.h"
#include "MAX3421.h"
#include "DescriptorParser.h"
#include "Console.h"

using namespace USBH;

namespace USBT {
    extern HUBDriver hubDriver;
}

HUBDriver::HUBDriver() {
    uint8_t i = 1;
    for (auto &item: Ports) {
        //item.HubDevice = this;
        item.PortNum = i++;
        item.Flags.IsLowSpeedDevice = false;
    }
}

struct __attribute__((packed)) HubDescriptor {
    uint8_t bDescriptorLength;
    uint8_t bDescriptorType;
    uint8_t bNumberOfPorts;
    uint16_t wHubCharacteristics;
    uint8_t bPowerOnToPowerGood;
    uint8_t bHubControlCurrent;
    uint8_t bRemoveAndPowerMask[64];
};

struct PortStatus {
    uint8_t PortConnection: 1;
    uint8_t PortEnabled: 1;
    uint8_t PortSuspend: 1;
    uint8_t PortOvercurrent: 1;
    uint8_t PortReset: 1;
    uint8_t Reserved: 3;
    uint8_t PortPower: 1;
    uint8_t PortLowSpeed: 1;
    uint8_t PortHighSpeed: 1;
    uint8_t PortTest: 1;
    uint8_t PortIndicator: 1;
};

task<bool>
HUBDriver::Analyze(USBTarget &Device, uint8_t DeviceClass, const USBH::DescriptorParser &Descriptor) {
    if (UsbAddress != 0)
        co_return false;

    auto Interface = Descriptor.Interface;
    auto EndPoint = Descriptor.EndPointIn;

    if (Interface == nullptr || EndPoint == nullptr)
        co_return false;

    if (DeviceClass == 0)
        DeviceClass = Interface->bInterfaceClass;

    if (DeviceClass != HID_CSCP_HubClass)
        co_return false;

    Endpoint = EndPoint->bEndpointAddress & 0xf;
    UsbAddress = Device.UsbAddress;
    Flags.IsLowSpeedDevice = Device.Flags.IsLowSpeedDevice;

    co_await SetConfiguration(Descriptor.Configuration->bConfigurationValue);
}

task<void> HUBDriver::Probe() {
    if (UsbAddress == 0 || Flags.IsProbed)
        co_return;

    Flags.IsProbed = true;

    auto req = USB_Request_Header_t{
            .bmRequestType = 0b10100000,
            .bRequest      = USB_REQUEST_GET_DESCRIPTOR,
            .wValue        = 0x29 << 8,
            .wIndex        = 0,
            .wLength       = 8
    };

    uint8_t buf[10];
    auto desc = (HubDescriptor *) co_await ControlRead(buf, req, 8, *this);

    if (desc == nullptr)
        co_return;

    portsNumber = desc->bNumberOfPorts;

    // Опрос всех портов
    for (uint8_t i = 0; i < portsNumber; ++i) {
        uint16_t portmask = 1 << i;

        // SET PORT_POWER
        req = USB_Request_Header_t
                {
                        .bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE |
                                          REQREC_ENDPOINT), // 0b00100011
                        .bRequest      = REQ_SetFeature,
                        .wValue        = 8,
                        .wIndex        = static_cast<uint16_t>(i + 1),
                        .wLength       = 0
                };

        co_await Control(req, *this);
        co_await USBH::Delay(50);

        // SET PORT_RESET
        req = {
                .bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE |
                                  REQREC_ENDPOINT), // 0b00100011
                .bRequest      = REQ_SetFeature,
                .wValue        = 4,
                .wIndex        = static_cast<uint16_t>(i + 1),
                .wLength       = 0
        };

        co_await Control(req, *this);
        co_await USBH::Delay(50);

        req = {
                .bmRequestType = (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE |
                                  REQREC_ENDPOINT), // 0b10100011
                .bRequest      = USB_REQUEST_GET_STATUS,
                .wValue        = 0,
                .wIndex        = static_cast<uint16_t>(i + 1),
                .wLength       = 4
        };

        auto status = (PortStatus *) co_await ControlRead(buf, req, 4, *this);

        if (status->PortConnection == 0)
            continue;

        CON << PSTR("HUB @") << DEC(UsbAddress) << PSTR(": ");

        if (status->PortLowSpeed)
            CON << PSTR("Low");
        else
            CON << PSTR("High");

        CON << PSTR(" speed device found at port ") << DEC(i) << endl();

        portsmask |= portmask;

        auto &p = Ports[i];

        p.Flags.IsLowSpeedDevice = status->PortLowSpeed;
        p.UsbAddress = 0;

        co_await USBH::InitializeNewDevice(p);
    }

    co_return;
}


task<bool> HUBDriver::Poll() {
    // Если состояние любого из портов изменилось - сброс стека
    for (uint8_t i = 0; i < portsNumber; ++i) {
        auto req = USB_Request_Header_t{
                .bmRequestType = (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE |
                                  REQREC_ENDPOINT), // 0b10100011
                .bRequest      = USB_REQUEST_GET_STATUS,
                .wValue        = 0,
                .wIndex        = static_cast<uint16_t>(i + 1),
                .wLength       = 4
        };

        uint8_t buf[8];
        auto status = (PortStatus *) co_await ControlRead(buf, req, 4, *this);

        if (status == nullptr)
            continue;

        if ((status->PortConnection != 0) != ((portsmask & (1 << i)) != 0))
            co_return false;
    }

    co_return true;
}

void HUBDriver::Reset() {
    UsbAddress = 0;
    Endpoint = 0;
    Flags.IsLowSpeedDevice = false;
    Flags.SndToggle = false;
    Flags.RcvToggle = false;
    Flags.IsProbed = false;
    portsNumber = 0;
    portsmask = 0;

    for (auto &item: Ports) {
        item.Flags.IsLowSpeedDevice = false;
        item.UsbAddress = 0;
    }
}

task<void> HubPortInfo::ResetPort() {
    auto addr = SPI::ReadUSB(PERADDR);
    auto mode = SPI::ReadUSB(MODE);

    auto req = USB_Request_Header_t{
            .bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE |
                              REQREC_ENDPOINT), // 0b00100011
            .bRequest      = REQ_SetFeature,
            .wValue        = 4,
            .wIndex        = PortNum,
            .wLength       = 0
    };

    //co_await HUBDriver::Control(req, *HubDevice);
    co_await HUBDriver::Control(req, USBT::hubDriver);

    SPI::WriteUSB(PERADDR, addr);
    SPI::WriteUSB(MODE, mode);
}