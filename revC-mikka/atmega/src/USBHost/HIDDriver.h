#ifndef CH_HIDDRIVER_H
#define CH_HIDDRIVER_H

#include "USBHost.h"
#include "HIDParser.h"

class HIDDriver : public USBH::USBTarget, public USBH::USBDriver, public HIDParser {
protected:
    uint8_t interface = 0;

    union {
        struct {
            uint8_t ReportId;
            uint8_t ReportSize;
        };

        struct {
            uint8_t ReportDescriptorType;
            uint16_t ReportDescriptorSize;
        };
    };

public:
    bool NewData = false;

    void Reset() override;

    USBH::task<bool>
    Analyze(USBTarget &Device, uint8_t DeviceClass, const USBH::DescriptorParser &Descriptor) override;

    USBH::task<void> Probe() override;

    virtual bool Check(const USBH::DescriptorParser &Descriptor) = 0;
};


#endif //CH_HIDDRIVER_H
