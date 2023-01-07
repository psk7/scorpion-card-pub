#ifndef CH_BOOTPROTOCOLKEYBOARD_H
#define CH_BOOTPROTOCOLKEYBOARD_H

#include "BootProtocolDriver.h"

class KeyboardDriver : public BootProtocolDriver {
public:
    uint8_t data[8] = {};
    uint8_t leds = 0;

    struct {
        bool NonConfigured: 1;
        bool Configured: 1;
    };

    KeyboardDriver() : NonConfigured(true), Configured(false) {}

    bool CheckInterface(USB_StdDescriptor_Interface_t *Interface) override;

    void OnData(uint8_t *Buffer) override;

    bool Poll() override;

    void OnComplete(uint8_t *Buffer, uint8_t Id, PortInfo &Port) override;

    void SetLeds(uint8_t Leds);
};

#endif //CH_BOOTPROTOCOLKEYBOARD_H
