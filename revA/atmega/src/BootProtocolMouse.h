#ifndef CH_BOOTPROTOCOLMOUSE_H
#define CH_BOOTPROTOCOLMOUSE_H

#include "BootProtocolDriver.h"

class MouseDriver : public BootProtocolDriver {
private:
    int16_t x = 0, y = 0;
    int8_t wh = 0, buttons = 0;

public:
    bool CheckInterface(USB_StdDescriptor_Interface_t *Interface) override;

    void OnData(uint8_t *Buffer) override;

    bool Poll() override;

    int16_t GetX() const;

    int16_t GetY() const;

    uint8_t GetButtons() const;

    int8_t GetWheel() const;
};

#endif //CH_BOOTPROTOCOLMOUSE_H
