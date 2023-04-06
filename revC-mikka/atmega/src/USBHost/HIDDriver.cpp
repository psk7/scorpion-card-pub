#include "HIDDriver.h"
#include "HIDClassCommon.h"
#include "usblib.h"
#include "USBHost.h"
#include "DescriptorParser.h"
#include "Console.h"

using namespace USBH;

task<bool>
HIDDriver::Analyze(USBTarget &Device, uint8_t DeviceClass, const USBH::DescriptorParser &Descriptor) {
    if (UsbAddress != 0)
        co_return false;

    CON << PSTR("HID probe @ ") << HEX(Device.UsbAddress) << endl();

    auto Interface = Descriptor.Interface;
    auto EndPoint = Descriptor.EndPointIn;
    auto HID = Descriptor.HidDescriptor;

    if (Interface == nullptr || EndPoint == nullptr || HID == nullptr)
        co_return false;

    if (DeviceClass == 0)
        DeviceClass = Interface->bInterfaceClass;

    if (DeviceClass != HID_CSCP_HIDClass)
        co_return false;

    if (!Check(Descriptor))
        co_return false;

    interface = Interface->bInterfaceNumber;

    auto confId = Descriptor.Configuration->bConfigurationValue;
    ReportDescriptorSize = HID->DescriptorList[0].wReportLength;
    ReportDescriptorType = HID->DescriptorList[0].bReportType;

    co_await SetConfiguration(confId);

    Endpoint = EndPoint->bEndpointAddress & 0xf;
    UsbAddress = Device.UsbAddress;
    Flags.IsLowSpeedDevice = Device.Flags.IsLowSpeedDevice;

    co_return true;
}

task<void> HIDDriver::Probe() {
    if (UsbAddress == 0 || Flags.IsProbed)
        co_return;

    Flags.IsProbed = true;

    auto reportDescriptorSize = ReportDescriptorSize;

    CON << PSTR("HID read descriptor len=") << DEC(reportDescriptorSize) << endl();

    // Чтение дескриптора HID
    auto req = USB_Request_Header_t{
            .bmRequestType = 0x81,
            .bRequest      = USB_REQUEST_GET_DESCRIPTOR,
            .wValue        = static_cast<uint16_t>(ReportDescriptorType << 8),
            .wIndex        = interface,
            .wLength       = reportDescriptorSize
    };

    co_await SetupControlReadAsync(req, *this);

    {
        uint8_t readed;
        auto reportDescriptor = ReceiveControlAsync(reportDescriptorSize, readed, true);

        CON << PSTR("HID Descriptor: ") << endl();

        HID_ReportInfo_t info;
        co_await ParseDescriptor(reportDescriptor, reportDescriptorSize, &info);

        while (readed < reportDescriptorSize)
            co_await reportDescriptor; // Чтобы чтение корректно закрылось

        ReportSize = USB_GetHIDReportSize(&info, ReportId, HID_REPORT_ITEM_In);

        if (info.UsingReportIDs)
            ++ReportSize;

        // reportDescriptorStream destroy here
    }

    CON << endl();

    if (!IsRecognized) {
        CON << PSTR("HID device is NOT recognized") << endl();

        Reset();
        co_return;
    }

    CON << PSTR("HID device is recognized") << endl();

    co_return;
}


void HIDDriver::Reset() {
    IsRecognized = false;
    UsbAddress = 0;
    Endpoint = 0;
    Flags.IsLowSpeedDevice = false;
    Flags.SndToggle = false;
    Flags.RcvToggle = false;
    Flags.IsProbed = false;
    interface = 0;
    NewData = false;
}

