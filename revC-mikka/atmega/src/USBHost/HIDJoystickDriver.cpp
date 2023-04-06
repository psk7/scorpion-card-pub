#include "HIDJoystickDriver.h"
#include "HIDClassCommon.h"
#include "usblib.h"
#include "USBHost.h"
#include "HIDReport.h"
#include "Joystick.h"
#include "DescriptorParser.h"
#include "Console.h"

using namespace USBH;

#define REPORT_RCV_BUFFER_SIZE 64

void HIDJoystickDriver::OnItem(const HID_ReportItem_t &CurrentItem) {
    for (HID_CollectionPath_t *CurrPath = CurrentItem.CollectionPath;
         CurrPath != nullptr; CurrPath = CurrPath->Parent) {
        auto isJoystick =
                (CurrPath->Usage.Page == USAGE_PAGE_GENERIC_DCTRL) && (CurrPath->Usage.Usage == USAGE_JOYSTICK);
        auto isGamepad = (CurrPath->Usage.Page == USAGE_PAGE_GENERIC_DCTRL) && (CurrPath->Usage.Usage == USAGE_GAMEPAD);

        if (isJoystick || isGamepad) {
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
        }

        if (CurrentItem.Attributes.Usage.Usage == USAGE_Y) {
            HasY = true;
            Y = CurrentItem;
        }

        if (CurrentItem.Attributes.Usage.Usage == USAGE_HAT_SWITCH) {
            HatSwitch = CurrentItem;
            HasHatSwitch = true;
        }
    }

    if ((CurrentItem.Attributes.Usage.Page == USAGE_PAGE_BUTTON))
        if (btncnt < sizeof btnids) {
            ReportId = CurrentItem.ReportID;
            btnids[btncnt++] = CurrentItem.BitOffset;
        }
}

task<bool> HIDJoystickDriver::Poll() {
    if (UsbAddress == 0)
        co_return true;

    uint8_t rbuf[REPORT_RCV_BUFFER_SIZE];
    auto buf = (uint8_t *) (co_await BulkIn(rbuf, ReportSize, *this));

    if (buf == nullptr)
        co_return true;

    if (PrintReports)
        CON << (char) 13 << PSTR("HID Joystick report: ") << Buffer(buf, ReportSize) << PSTR("  ");

    int bit = 1 << 4;

    bits = 0;

    for (const auto &item: btnids)
        if (item != 0xff) {
            if (GetItemInfo(buf, HID_ReportItem(item, ReportId)))
                bits |= bit;

            bit <<= 1;
        }

    if (HasX) {
        auto x = (uint8_t) GetItemInfo(buf, X);

        if (x == X.log_min)
            bits |= JOYSTICK_LEFT;
        if (x == X.log_max)
            bits |= JOYSTICK_RIGHT;
    }

    if (HasY) {
        auto y = (uint8_t) GetItemInfo(buf, Y);

        if (y == Y.log_min)
            bits |= JOYSTICK_UP;
        if (y == Y.log_max)
            bits |= JOYSTICK_DOWN;
    }

    static const uint8_t Directions[] PROGMEM = {
            JOYSTICK_UP, JOYSTICK_UP | JOYSTICK_RIGHT, JOYSTICK_RIGHT, JOYSTICK_RIGHT | JOYSTICK_DOWN,
            JOYSTICK_DOWN, JOYSTICK_LEFT | JOYSTICK_DOWN, JOYSTICK_LEFT, JOYSTICK_UP | JOYSTICK_LEFT
    };

    if (HasHatSwitch) {
        uint16_t lv = GetItemInfo(buf, HatSwitch, false);
        auto deg = (int16_t) (lv * HatSwitch.k);

        if (deg < 360) {
            if (deg > 315)
                deg -= 360;

            for (uint8_t i = 0; i <= 7; ++i) {
                auto zMin = (int16_t) (i * 45 - 22);
                auto zMax = (int16_t) (i * 45 + 23);

                if (deg >= zMin && deg < zMax)
                    bits |= pgm_read_byte(&Directions[i]);
            }
        }
    }

    co_return true;
}

void HIDJoystickDriver::Reset() {
    HIDDriver::Reset();
    bits = {};
    btncnt = 0;
    memset(&btnids, 0xff, sizeof btnids);
    X = HID_ReportItem();
    Y = HID_ReportItem();
    HasHatSwitch = false;
}

uint16_t HIDJoystickDriver::GetBits() {
    return UsbAddress != 0 ? bits : 0;
}
