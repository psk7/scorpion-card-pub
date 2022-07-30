#include "BootProtocolKeyboard.h"
#include "HIDClassCommon.h"
#include "StdRequestType.h"

#ifdef WIN32

#include <iostream>

#else
#endif

bool KeyboardDriver::CheckInterface(USB_StdDescriptor_Interface_t *Interface) {
    return Interface->bInterfaceSubClass != HID_CSCP_BootSubclass ||
           Interface->bInterfaceProtocol != HID_CSCP_KeyboardBootProtocol;
}

void KeyboardDriver::OnData(uint8_t *Buffer) {
    if (Buffer == nullptr)
        return;     // NACK

    memcpy(&data, Buffer, sizeof data);
    NewData = true;
}

bool KeyboardDriver::Poll() {
    auto r = BootProtocolDriver::Poll();

    if (r) {
        if (!IsProtocolEnabled) {
            auto req = USB_Request_Header_t
                    {
                            .bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE),
                            .bRequest      = HID_REQ_SetProtocol,
                            .wValue        = 0,
                            .wIndex        = 0,
                            .wLength       = 0,
                    };

            RequestControl((uint8_t *) &req, 7, *this);

            IsProtocolEnabled = true;
            return true;
        }

        uint8_t cmd[] = {0b00100001, 0x09, 0, 2, 0, 0, 1, 0};
        uint8_t d[] = {leds};
        RequestControlWrite(cmd, sizeof d, &d[0], 8, *this);
    }

    return r;
}

void KeyboardDriver::OnComplete(uint8_t *Buffer, uint8_t Id, PortInfo &Port) {
    switch (Id) {
        case 7:
            CompletePoll();
            break;

        case 8:
            RequestBulkIn(8, 0x80, *this);
            break;

        default:
            break;
    }
}

void KeyboardDriver::SetLeds(uint8_t Leds) {
    leds = Leds;
}
