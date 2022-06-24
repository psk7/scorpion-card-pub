#include "BootProtocolMouse.h"
#include "HIDClassCommon.h"

bool MouseDriver::CheckInterface(USB_StdDescriptor_Interface_t *Interface) {
    return Interface->bInterfaceSubClass != HID_CSCP_BootSubclass ||
           Interface->bInterfaceProtocol != HID_CSCP_MouseBootProtocol;
}

void MouseDriver::OnData(uint8_t *Buffer) {
    if (Buffer == nullptr)
        return;

    x += (int16_t) (int8_t)Buffer[1];
    y -= (int16_t) (int8_t)Buffer[2];

    buttons = Buffer[0];
    wh += (int8_t)Buffer[3];

    NewData = true;
}

bool MouseDriver::Poll() {
    auto r = BootProtocolDriver::Poll();
    if (r)
        RequestBulkIn(4, 0x80, *this);

    return r;
}

int16_t MouseDriver::GetX() const {
    return x;
}

int16_t MouseDriver::GetY() const {
    return y;
}

uint8_t MouseDriver::GetButtons() const {
    return buttons;
}

int8_t MouseDriver::GetWheel() const {
    return wh;
}
