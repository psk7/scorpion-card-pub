#ifndef CH_HIDDRIVER_H
#define CH_HIDDRIVER_H

#include "USBHost.h"
#include "HIDParser.h"

class HIDJoystickDriver : public USBH::USBTarget, public USBH::USBDriver, public HIDParser {
private:
    HID_ReportItem X{}, Y{};

    uint16_t bits{};

    uint8_t btncnt = 0;
    uint8_t btnids[12]{};

    uint8_t ReportSize = 0;

public:
    void Reset() override;

    USBH::task<bool>
    Analyze(USBTarget &Device, uint8_t DeviceClass, USB_StdDescriptor_Configuration_Header_t *Configuration,
            USB_StdDescriptor_Interface_t *Interface) override;

    USBH::task<bool> Poll() override;

    void OnItem(const HID_ReportItem_t &CurrentItem) override;

    uint16_t GetBits();
};


#endif //CH_HIDDRIVER_H
