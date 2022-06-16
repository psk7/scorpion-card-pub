#ifndef CH_HUBDRIVER_H
#define CH_HUBDRIVER_H

#include "USBHost.h"

class HUBDriver;

class HubPortInfo : public PortInfo {
public:
    HUBDriver *driver;
    uint8_t PortNum;
};

class HUBDriver : public USBDriver, public USBTarget {
private:
    HubPortInfo Ports[4] = {};
    HubPortInfo *CurrentPortInfo = &Ports[0];

public:
    HUBDriver();

    bool Analyze(uint8_t Address, bool IsLowSpeed, uint8_t DeviceClass,
                 USB_StdDescriptor_Configuration_Header_t *Configuration,
                 USB_StdDescriptor_Interface_t *Interface) override;

    void Reset() override;

    void OnComplete(uint8_t *Buffer, uint8_t Id, PortInfo &Port) override;

    void OnBulkComplete(uint8_t *Buffer, uint8_t Id, uint8_t Address) override;

    bool Poll() override;
};


#endif //CH_HUBDRIVER_H
