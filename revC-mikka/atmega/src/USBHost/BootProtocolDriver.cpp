#include "BootProtocolDriver.h"
#include "HIDClassCommon.h"
#include "usblib.h"
#include "USBHost.h"
#include "DescriptorParser.h"

using namespace USBH;

task<bool>
BootProtocolDriver::Analyze(USBTarget &Device, uint8_t DeviceClass, const USBH::DescriptorParser &Descriptor) {
    if (UsbAddress != 0)
        co_return false;

    auto Interface = Descriptor.Interface;
    auto EndPoint = Descriptor.EndPointIn;

    if (Interface == nullptr || EndPoint == nullptr)
        co_return false;

    if (DeviceClass == 0)
        DeviceClass = Interface->bInterfaceClass;

    if (DeviceClass != HID_CSCP_HIDClass)
        co_return false;

    if (CheckInterface(Interface))
        co_return false;

    Endpoint = EndPoint->bEndpointAddress & 0xf;
    UsbAddress = Device.UsbAddress;
    Flags.IsLowSpeedDevice = Device.Flags.IsLowSpeedDevice;
    interface = Interface->bInterfaceNumber;

    co_await SetConfiguration(Descriptor.Configuration->bConfigurationValue);
    co_return true;
}

void BootProtocolDriver::Reset() {
    UsbAddress = 0;
    Endpoint = 0;
    Flags.IsLowSpeedDevice = false;
    Flags.SndToggle = false;
    Flags.RcvToggle = false;
    NewData = false;
    interface = 0;
}
