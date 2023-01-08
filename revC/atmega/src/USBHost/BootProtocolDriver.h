#ifndef CH_BOOTPROTOCOLDRIVER_H
#define CH_BOOTPROTOCOLDRIVER_H

#include "USBHost.h"

class BootProtocolDriver : public USBH::USBDriver, public USBH::USBTarget {
public:
    bool NewData = false;

    USBH::task<bool>
    Analyze(USBTarget &Device, uint8_t DeviceClass, USB_StdDescriptor_Configuration_Header_t *Configuration,
            USB_StdDescriptor_Interface_t *Interface) override;

    void Reset() override;

    virtual bool CheckInterface(USB_StdDescriptor_Interface_t *Interface) = 0;
};

#endif //CH_BOOTPROTOCOLDRIVER_H
