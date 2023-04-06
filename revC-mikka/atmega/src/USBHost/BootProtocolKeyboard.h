#ifndef CH_BOOTPROTOCOLKEYBOARD_H
#define CH_BOOTPROTOCOLKEYBOARD_H

#include "BootProtocolDriver.h"

class KeyboardDriver : public BootProtocolDriver {
private:
    uint8_t leds = 0;
    uint8_t ledsupdcnt = 0;

public:
    uint8_t data[8] = {};

    struct {
        bool NonConfigured: 1;
        bool Configured: 1;
        bool LedsUpdated: 1;
    };

    KeyboardDriver() : NonConfigured(true), Configured(false), LedsUpdated(false) {}

    bool CheckInterface(USB_StdDescriptor_Interface_t *Interface) override;

    USBH::task<bool> Poll() override;

    void SetLeds(uint8_t Leds);

    void Reset() override;
};

#endif //CH_BOOTPROTOCOLKEYBOARD_H
