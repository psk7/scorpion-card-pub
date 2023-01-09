#include "BootProtocolDriver.h"
#include "HIDClassCommon.h"
#include "USBHost.h"

using namespace USBH;

task<bool>
BootProtocolDriver::Analyze(USBTarget &Device, uint8_t DeviceClass,
                            USB_StdDescriptor_Configuration_Header_t *Configuration,
                            USB_StdDescriptor_Interface_t *Interface) {

    if (UsbAddress != 0)
        co_return false;

    if (DeviceClass == 0)
        DeviceClass = Interface->bInterfaceClass;

    if (DeviceClass != HID_CSCP_HIDClass)
        co_return false;

    if (CheckInterface(Interface))
        co_return false;

    auto *ptr = (uint8_t *) Interface;

    while (true) {
        if (ptr[1] == 5)
            break;

        ptr += ptr[0];
    }

    auto epid = (USB_StdDescriptor_Endpoint_t*) ptr;

    Endpoint = epid->bEndpointAddress & 0xf;
    UsbAddress = Device.UsbAddress;
    Flags.IsLowSpeedDevice = Device.Flags.IsLowSpeedDevice;

    co_await SetConfiguration(Configuration->bConfigurationValue);
    co_return true;
}

void BootProtocolDriver::Reset() {
    UsbAddress = 0;
    Endpoint = 0;
    Flags.IsLowSpeedDevice = false;
    Flags.SndToggle = false;
    Flags.RcvToggle = false;
    NewData = false;
}
