#include "HUBDriver.h"
#include "HIDClassCommon.h"

HUBDriver::HUBDriver() {
    uint8_t i = 1;
    for (auto &item: Ports) {
        item.driver = this;
        item.PortNum = i++;
        item.Flags = 0;
    }
}

bool HUBDriver::Analyze(uint8_t Address, bool IsLowSpeed, uint8_t DeviceClass,
                        USB_StdDescriptor_Configuration_Header_t *Configuration,
                        USB_StdDescriptor_Interface_t *Interface) {
    if (DeviceClass == 0)
        DeviceClass = Interface->bInterfaceClass;

    if (DeviceClass != HID_CSCP_HubClass)
        return false;

    auto *ptr = (uint8_t *) Interface;

    while (true) {
        if (ptr[1] == 5)
            break;

        ptr += ptr[0];
    }

    auto epid = (USB_StdDescriptor_Endpoint_t *) ptr;

    Endpoint = epid->bEndpointAddress & 0xf;
    UsbAddress = Address;
    Flags.IsLowSpeedDevice = IsLowSpeed;

    SetConfiguration(Configuration->bConfigurationValue);
    return true;
}

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

void HUBDriver::OnComplete(uint8_t *Buffer, uint8_t Id, PortInfo &Port) {
    switch (Id) {
        case 1:
            // SET PORT_POWER
            RequestDelay(50, 2);
            return;

        case 2: {
            // SET PORT_RESET
            uint8_t c[] = {0b00100011, 3, 4, 0, CurrentPortInfo->PortNum, 0, 0, 0};
            RequestControl(&c[0], 3, *this);
            return;
        }

        case 3:
            RequestDelay(50, 4);
            return;

        case 4: {
            uint8_t c[8] = {0b10100011, 0, 0, 0, CurrentPortInfo->PortNum, 0, 4, 0};
            RequestControlRead(c, 4, 5, *this);
            return;
        }

        case 5: {
            auto status = (PortStatus *) Buffer;

            CurrentPortInfo->LowSpeed = status->PortLowSpeed;
            CurrentPortInfo->Address = 0;

            if (status->PortConnection) {
                SetPortInfo(CurrentPortInfo);
                EnableNewDeviceDiscovery();
            } else {
                CurrentPortInfo->Configuring = false;
                CurrentPortInfo->Configured = true;
            }
            CurrentPortInfo->Connected = status->PortConnection;

            CompletePoll();
            return;
        }

        case 6: {
            // Check port surprisingly connected or disconnected
            auto status = (PortStatus *) Buffer;

            if (CurrentPortInfo->Configured && (CurrentPortInfo->Connected != status->PortConnection))
                // Port connected or disconnected
                ResetUsbStack();
            else
                CompletePoll();

            return;
        }
    }
}

bool HUBDriver::Poll() {

    // PORT_POWER_FEATURE = 8
    // PORT_ENABLE_FEATURE = 1
    // PORT_RESET_FEATURE = 4
    // SET_FEATURE = 3

    if (UsbAddress == 0)
        return false;

    if (CurrentPortInfo->Configuring)
        return false;

    if (CurrentPortInfo->Configured) {
        // Switch port
        if (CurrentPortInfo == &Ports[sizeof Ports / sizeof(Ports[0]) - 1])
            CurrentPortInfo = &Ports[0];
        else
            CurrentPortInfo++;

        if (CurrentPortInfo->Configured) {
            // Recheck port for connection/disconnection
            uint8_t c[8] = {0b10100011, 0, 0, 0, CurrentPortInfo->PortNum, 0, 4, 0};
            RequestControlRead(c, 4, 6, *this);
            return true;
        }
    }

    DisableNewDeviceDiscovery();

    CurrentPortInfo->Configuring = true;

    // SET PORT_POWER
    uint8_t c[] = {0b00100011, 3, 8, 0, CurrentPortInfo->PortNum, 0, 0, 0};
    RequestControl(&c[0], 1, *this);
    return true;
}

void HUBDriver::OnBulkComplete(uint8_t *Buffer, uint8_t Id, uint8_t Address) {}

void HUBDriver::Reset() {
    UsbAddress = 0;
    Endpoint = 0;
    Flags.IsLowSpeedDevice = false;
    Flags.SndToggle = false;
    Flags.RcvToggle = false;
    CurrentPortInfo = &Ports[0];

    for (auto &item: Ports) {
        item.Flags = 0;
        item.Address = 0;
    }
}
