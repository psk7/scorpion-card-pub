#include "HIDMouseDriver.h"
#include "HIDClassCommon.h"
#include "usblib.h"
#include "USBHost.h"
#include "HIDReport.h"
#include "DescriptorParser.h"
#include "Console.h"

using namespace USBH;

#define REPORT_RCV_BUFFER_SIZE 64

bool HIDMouseDriver::Check(const DescriptorParser &Descriptor) {
    auto Interface = Descriptor.Interface;

    if (Interface == nullptr)
        return false;

    if (Interface->bInterfaceSubClass != HID_CSCP_BootSubclass ||
        Interface->bInterfaceProtocol != HID_CSCP_MouseBootProtocol)
        return false;

    return true;
}

extern "C" unsigned char __heap_start; // NOLINT(bugprone-reserved-identifier)

void HIDMouseDriver::OnItem(const HID_ReportItem_t &CurrentItem) {
    for (HID_CollectionPath_t *CurrPath = CurrentItem.CollectionPath;
         CurrPath != nullptr; CurrPath = CurrPath->Parent) {
        if ((CurrPath->Usage.Page == USAGE_PAGE_GENERIC_DCTRL) && (CurrPath->Usage.Usage == USAGE_MOUSE)) {
            IsRecognized = true;
            break;
        }
    }

    if (!IsRecognized)
        return;

    if ((CurrentItem.Attributes.Usage.Page == USAGE_PAGE_GENERIC_DCTRL)) {
        if (CurrentItem.Attributes.Usage.Usage == USAGE_X) {
            HasX = true;
            X = CurrentItem;
            ReportId = X.report_id;
        }

        if (CurrentItem.Attributes.Usage.Usage == USAGE_Y) {
            HasY = true;
            Y = CurrentItem;
        }

        if (CurrentItem.Attributes.Usage.Usage == USAGE_WHEEL) {
            HasWheel = true;
            Wheel = CurrentItem;
        }
    }

    if ((CurrentItem.Attributes.Usage.Page == USAGE_PAGE_BUTTON))
        if (btncnt < sizeof btnids) {
            ButtonsMask |= (1 << btncnt);
            btnids[btncnt++] = CurrentItem.BitOffset;
            ReportId = CurrentItem.ReportID;
        }
}

task<bool> HIDMouseDriver::Poll() {
    if (UsbAddress == 0 || (ReportSize > REPORT_RCV_BUFFER_SIZE))
        co_return true;

    if (!IsRecognized || !HasX || !HasY)
        co_return true;

    if (!Configured) {
        auto req = USB_Request_Header_t{
                .bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE),
                .bRequest      = HID_REQ_SetProtocol,
                .wValue        = 1,
                .wIndex        = interface,
                .wLength       = 0,
        };

        co_await Control(req, *this);

        co_await Delay(2);

        req = USB_Request_Header_t{
                .bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE),
                .bRequest      = HID_REQ_SetIdle,
                .wValue        = 0,
                .wIndex        = interface,
                .wLength       = 0,
        };

        co_await Control(req, *this);

        co_await Delay(2);

        Configured = true;
    }

    uint8_t rbuf[REPORT_RCV_BUFFER_SIZE];
    auto buf = (uint8_t *) (co_await BulkIn(rbuf, ReportSize, *this));

    NewData = true;

    if (buf == nullptr)
        co_return true;

    if (PrintReports)
        CON << PSTR("HID report (len=") << HEX(ReportSize) << PSTR("): ") << Buffer(buf, ReportSize) << endl();

    auto dX = (uint16_t) GetItemInfo(buf, X);
    auto dY = (uint16_t) GetItemInfo(buf, Y);
    auto dWheel = HasWheel ? (uint8_t) GetItemInfo(buf, Wheel) : (uint8_t) 0;

    buttons = 0;

    for (uint8_t i = 0; i < (sizeof btnids / sizeof btnids[0]); ++i) {
        if (ButtonsMask & (1 << i))
            buttons |= GetItemInfo(buf, HID_ReportItem(btnids[i], ReportId)) != 0 ? (1 << i) : 0;
    }

    x += dX;
    y -= dY;

    wh += dWheel;

    co_return true;
}

int16_t HIDMouseDriver::GetX() const {
    return (int16_t) x;
}

int16_t HIDMouseDriver::GetY() const {
    return (int16_t) y;
}

uint8_t HIDMouseDriver::GetButtons() const {
    return buttons;
}

int8_t HIDMouseDriver::GetWheel() const {
    return (int8_t) wh;
}

void HIDMouseDriver::Reset() {
    HIDDriver::Reset();
    X = HID_ReportItem();
    Y = HID_ReportItem();
    Wheel = HID_ReportItem();
    memset(&btnids, 0, sizeof btnids);
    btncnt = 0;
    Configured = false;
    HasWheel = false;
    HasX = false;
    HasY = false;
    ButtonsMask = 0;
}
