#ifndef CH_HUBDRIVER_H
#define CH_HUBDRIVER_H

#include "USBHost.h"
#include "DescriptorParser.h"

class HUBDriver;

class HubPortInfo : public USBH::PortInfo {
public:
    //USBH::USBTarget *HubDevice{};
    uint8_t PortNum{};

    USBH::task<void> ResetPort() override;
};

class HUBDriver : public USBH::USBDriver, public USBH::USBTarget {
private:
    HubPortInfo Ports[8] = {};
    uint8_t portsNumber = 0;
    uint16_t portsmask = 0;

public:
    HUBDriver();

    USBH::task<bool>
    Analyze(USBTarget &Device, uint8_t DeviceClass, const USBH::DescriptorParser &Descriptor) override;

    USBH::task<void> Probe() override;

    void Reset() override;

    USBH::task<bool> Poll() override;
};


#endif //CH_HUBDRIVER_H
