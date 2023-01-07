#include "HIDDriver.h"
#include "HIDClassCommon.h"

HIDDriver::HIDDriver() {
    memset(&Ports, 0, sizeof Ports);
}

bool HIDDriver::Analyze(uint8_t Address, bool IsLowSpeed, uint8_t DeviceClass,
                        USB_StdDescriptor_Configuration_Header_t *Configuration,
                        USB_StdDescriptor_Interface_t *Interface) {

    if (DeviceClass == 0)
        DeviceClass = Interface->bInterfaceClass;

    if (DeviceClass != HID_CSCP_HIDClass)
        return false;

    auto *ptr = (uint8_t *) Interface;

    while (true) {
        if (ptr[1] == 33)
            break;

        ptr += ptr[0];
    }

    auto hid = (PHID_DESCRIPTOR) ptr;

    if (hid->DescriptorList[0].wReportLength > USB_BUFFER_SIZE)
        return false;

    ConfigurationId = Configuration->bConfigurationValue;
    ReportDescriptorSize = (uint8_t) hid->DescriptorList[0].wReportLength;

    uint8_t c[] = {0x81, 0x06, 0x00, hid->DescriptorList[0].bReportType, 0x00, 0x00, ReportDescriptorSize, 0x00};

    //SetConfiguration(Configuration->bConfigurationValue);
    RequestControlRead(c, ReportDescriptorSize, 1);
    return true;
}

void HIDDriver::RegisterPort(HIDMiniport &Port) {
    for (auto &item: Ports)
        if (item == nullptr) {
            item = &Port;
            break;
        }
}

void HIDDriver::OnComplete(uint8_t *Buffer, uint8_t Id, PortInfo &Port) {
    switch (Id) {
        case 1:
            for (const auto &item: Ports) {
                if (item == nullptr)
                    continue;

                USBTarget *tgt;
                if ((tgt = item->Probe(Port.Address, Buffer, ReportDescriptorSize)) != nullptr) {
                    // Device accepted
                    tgt->UsbAddress = Port.Address;
                    tgt->Flags.IsLowSpeedDevice = Port.LowSpeed;
                    SetConfiguration(ConfigurationId);
                    return;
                }
            }

            CompleteEnumeration();
            return;

        case 0x80:
            CompletePoll();
            return;
    }
}

void HIDDriver::OnBulkComplete(uint8_t *Buffer, uint8_t Id, uint8_t Address) {
    CompletePoll();

    if (Id != 0x80)
        return;

    for (const auto &item: Ports)
        if (item != nullptr)
            item->OnComplete(Address, Buffer);
}

bool HIDDriver::Poll() {
    auto port = Ports[PollingId++];

    if (PollingId >= (sizeof(Ports) / sizeof(Ports[0])))
        PollingId = 0;

    if (port != nullptr)
        return port->Poll(*this);

    return false;
}

void HIDDriver::Reset() {
    ReportDescriptorSize = 0;
    ConfigurationId = 0;
    PollingId = 0;

    for (const auto &item: Ports)
        if (item != nullptr)
            item->Reset();
}

