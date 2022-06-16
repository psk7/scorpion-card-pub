#include "BootProtocolMouse.h"
#include "HIDClassCommon.h"

bool MouseDriver::CheckInterface(USB_StdDescriptor_Interface_t *Interface) {
    return Interface->bInterfaceSubClass != HID_CSCP_BootSubclass ||
           Interface->bInterfaceProtocol != HID_CSCP_MouseBootProtocol;
}

void MouseDriver::OnData(uint8_t *Buffer) {
    if (Buffer == nullptr)
        return;

    int8_t dx = Buffer[1];
    int8_t dy = Buffer[2];
    int8_t dw = Buffer[3];

    data[0] = Buffer[0];
    data[1] += dx;
    data[2] += dy;
    data[3] += dw;

    NewData = true;
}

bool MouseDriver::Poll() {
    auto r = BootProtocolDriver::Poll();
    if (r)
        RequestBulkIn(4, 0x80, *this);

    return r;
}
