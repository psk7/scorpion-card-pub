#ifndef CH_BOOTPROTOCOLDRIVER_H
#define CH_BOOTPROTOCOLDRIVER_H

#include "USBHost.h"

class BootProtocolDriver : public USBDriver, public USBTarget {
public:
    bool NewData = false;

    bool Analyze(uint8_t Address, bool IsLowSpeed, uint8_t DeviceClass,
                 USB_StdDescriptor_Configuration_Header_t *Configuration,
                 USB_StdDescriptor_Interface_t *Interface) override;

    void Reset() override;

    void OnComplete(uint8_t *Buffer, uint8_t Id, PortInfo &Port) override;

    void OnBulkComplete(uint8_t *Buffer, uint8_t Id, uint8_t Address) override;

    bool Poll() override;

    virtual bool CheckInterface(USB_StdDescriptor_Interface_t *Interface) = 0;

    virtual void OnData(uint8_t *Buffer) = 0;
};

#endif //CH_BOOTPROTOCOLDRIVER_H
