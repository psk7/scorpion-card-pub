#ifndef CH_BOOTPROTOCOLMOUSE_H
#define CH_BOOTPROTOCOLMOUSE_H

#include "BootProtocolDriver.h"

class MouseDriver : public BootProtocolDriver {
public:
    uint8_t data[4];
    bool NewData = false;

    bool CheckInterface(USB_StdDescriptor_Interface_t *Interface) override;

    void OnData(uint8_t *Buffer) override;

    bool Poll() override;

    uint8_t GetX();

    uint8_t GetY();

    uint8_t GetButtons();

    uint8_t GetWheel();
};

#endif //CH_BOOTPROTOCOLMOUSE_H
