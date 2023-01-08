#include "BootProtocolKeyboard.h"
#include "HIDClassCommon.h"
#include "StdRequestType.h"

using namespace USBH;

bool KeyboardDriver::CheckInterface(USB_StdDescriptor_Interface_t *Interface) {
    return Interface->bInterfaceSubClass != HID_CSCP_BootSubclass ||
           Interface->bInterfaceProtocol != HID_CSCP_KeyboardBootProtocol;
}

task<bool> KeyboardDriver::Poll() {
    if (UsbAddress == 0)
        co_return true;

    if (NonConfigured) {
        NonConfigured = false;

        auto req = USB_Request_Header_t{
                .bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE),
                .bRequest      = HID_REQ_SetProtocol,
                .wValue        = 0,
                .wIndex        = 0,
                .wLength       = 0,
        };

        co_await Control(req, *this);

        req = {
                .bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE),
                .bRequest      = HID_REQ_SetIdle,
                .wValue        = 0,
                .wIndex        = 0,
                .wLength       = 0,
        };

        co_await Control(req, *this);

        req = {
                .bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE),
                .bRequest      = HID_REQ_SetReport,
                .wValue        = 0x200,
                .wIndex        = 0,
                .wLength       = 1,
        };

        uint8_t d[] = {7};
        co_await ControlWrite(req, sizeof d, &d[0], *this);

        co_await USBH::Delay(250);

        uint8_t d2[] = {0};
        co_await ControlWrite(req, sizeof d2, &d2[0], *this);

        SetLeds(0);
        Configured = true;

        co_return true;
    }

    if (Configured) {
        if (LedsUpdated || ((ledsupdcnt++ % 16) == 0)) {
            LedsUpdated = false;

            auto req = USB_Request_Header_t{
                    .bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE),
                    .bRequest      = HID_REQ_SetReport,
                    .wValue        = 0x200,
                    .wIndex        = 0,
                    .wLength       = 1,
            };

            uint8_t d[] = {leds};
            co_await ControlWrite(req, sizeof d, &d[0], *this);
        }

        auto buf = co_await BulkIn(8, *this);

        if (buf != nullptr) {   // buf == nullptr -> NACK
            memcpy(&data, buf, sizeof data);
            NewData = true;
        }
    }

    co_return true;
}

void KeyboardDriver::SetLeds(uint8_t Leds) {
    LedsUpdated |= (leds != Leds);
    leds = Leds;
}

void KeyboardDriver::Reset() {
    BootProtocolDriver::Reset();

    NonConfigured = true;
    Configured = false;
    LedsUpdated = false;
    leds = 0;
}
