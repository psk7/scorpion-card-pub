#ifndef CH_HIDDRIVER_H
#define CH_HIDDRIVER_H

#include "USBHost.h"

class HIDMiniport {
public:
    virtual USBTarget *Probe(uint8_t Address, uint8_t *ReportDescriptor, uint8_t ReportDescriptorSize) = 0;

    virtual void Reset() = 0;

    virtual bool Poll(USBDriver &Driver) = 0;

    virtual void OnComplete(uint8_t Address, uint8_t *Buffer) = 0;
};

class HIDDriver : public USBDriver {
private:
    HIDMiniport *Ports[1];
    uint8_t ReportDescriptorSize = 0;
    uint8_t ConfigurationId = 0;
    uint8_t PollingId = 0;

public:
    HIDDriver();

    void Reset() override;

    void RegisterPort(HIDMiniport &Port);

    bool Analyze(uint8_t Address, bool IsLowSpeed, uint8_t DeviceClass,
                 USB_StdDescriptor_Configuration_Header_t *Configuration,
                 USB_StdDescriptor_Interface_t *Interface) override;

    void OnComplete(uint8_t *Buffer, uint8_t Id, PortInfo &Port) override;

    void OnBulkComplete(uint8_t *Buffer, uint8_t Id, uint8_t Address) override;

    bool Poll() override;
};


#endif //CH_HIDDRIVER_H
