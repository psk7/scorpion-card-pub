#include "BootProtocolDriver.h"

#include "HIDClassCommon.h"

bool
BootProtocolDriver::Analyze(uint8_t Address, bool IsLowSpeed, uint8_t DeviceClass,
                            USB_StdDescriptor_Configuration_Header_t *Configuration,
                            USB_StdDescriptor_Interface_t *Interface) {

    if (DeviceClass == 0)
        DeviceClass = Interface->bInterfaceClass;

    if (DeviceClass != HID_CSCP_HIDClass)
        return false;

    if (CheckInterface(Interface))
        return false;

    auto *ptr = (uint8_t *) Interface;

    while (true) {
        if (ptr[1] == 5)
            break;

        ptr += ptr[0];
    }

    auto epid = (USB_StdDescriptor_Endpoint_t*) ptr;

    Endpoint = epid->bEndpointAddress & 0xf;
    UsbAddress = Address;
    Flags.IsLowSpeedDevice = IsLowSpeed;

    SetConfiguration(Configuration->bConfigurationValue);
    return true;
}

void BootProtocolDriver::OnBulkComplete(uint8_t *Buffer, uint8_t Id, uint8_t Address) {
    CompletePoll();

    if (Id != 0x80)
        return;

    OnData(Buffer);
}

bool BootProtocolDriver::Poll() {
    if (UsbAddress == 0)
        return false;

    return true;
}

void BootProtocolDriver::OnComplete(uint8_t *Buffer, uint8_t Id, PortInfo &Port) {}

void BootProtocolDriver::Reset() {
    UsbAddress = 0;
    Endpoint = 0;
    Flags.IsLowSpeedDevice = false;
    Flags.SndToggle = false;
    Flags.RcvToggle = false;
}
