#ifndef CH_HIDJOYDRIVER_H
#define CH_HIDJOYDRIVER_H

#include "HIDDriver.h"

class HIDJoystickDriver : public HIDDriver {
private:
    HID_ReportItem X{}, Y{}, HatSwitch{};

    uint16_t bits{};

    uint8_t btncnt = 0;
    uint8_t btnids[12]{};

    struct {
        bool HasX : 1;
        bool HasY : 1;
        bool HasHatSwitch : 1;
    };

public:
    void Reset() override;

    USBH::task<bool> Poll() override;

    void OnItem(const HID_ReportItem_t &CurrentItem) override;

    uint16_t GetBits();

    bool Check(const USBH::DescriptorParser &Descriptor) override { return true; }
};

#endif //CH_HIDJOYDRIVER_H
