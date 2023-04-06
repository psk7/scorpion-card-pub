#ifndef CH_HIDMOUSEDRIVER_H
#define CH_HIDMOUSEDRIVER_H

#include "HIDDriver.h"

class HIDMouseDriver : public HIDDriver {
private:
    HID_ReportItem X{}, Y{}, Wheel{};

    uint16_t x = 0xaa, y = 0x55;
    uint8_t wh = 0;
    uint8_t buttons = 0;

    uint8_t btncnt = 0;
    uint8_t btnids[3]{};
    uint8_t ButtonsMask = 0;

    struct {
        bool HasX: 1;
        bool HasY: 1;
        bool HasWheel: 1;
        bool Configured: 1;
    };

public:
    void Reset() override;

    bool Check(const USBH::DescriptorParser &Descriptor) override;

    USBH::task<bool> Poll() override;

    void OnItem(const HID_ReportItem_t &CurrentItem) override;

    int16_t GetX() const;

    int16_t GetY() const;

    uint8_t GetButtons() const;

    int8_t GetWheel() const;
};


#endif //CH_HIDMOUSEDRIVER_H
