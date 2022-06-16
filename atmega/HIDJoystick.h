#ifndef CH_HIDJOYSTICK_H
#define CH_HIDJOYSTICK_H

#include "HIDParser.h"
#include "HIDDriver.h"

struct JoystickFDO : public USBTarget {
    HID_ReportItem X, Y;

    uint8_t bits{};

    uint8_t btncnt = 0;
    uint8_t btnids[16]{};

    void Reset();
};

class JoystickMiniport : public HIDMiniport, public HIDParser {
private:
    JoystickFDO Joystick;

public:
    USBTarget * Probe(uint8_t Address, uint8_t *ReportDescriptor, uint8_t ReportDescriptorSize) override;

    void Reset() override;

    void OnItem(const HID_ReportItem_t &CurrentItem) override;

    bool Poll(USBDriver &Driver) override;

    void OnComplete(uint8_t Address, uint8_t *Buffer) override;

    uint8_t GetBits();
};

#endif //CH_HIDJOYSTICK_H
