#include "HIDJoystick.h"
#include "HIDReport.h"

#ifdef WIN32
#include <iostream>
#else
#endif

USBTarget *JoystickMiniport::Probe(uint8_t Address, uint8_t *ReportDescriptor, uint8_t ReportDescriptorSize) {
    HID_ReportInfo_t info;
    ParseDescriptor(ReportDescriptor, ReportDescriptorSize, &info);

    if (!IsRecognized)
        return nullptr;

    Joystick.Endpoint = 1;
    return &Joystick;
}

void JoystickMiniport::OnItem(const HID_ReportItem_t &CurrentItem) {
    /* Iterate through the item's collection path, until either the root collection node or a collection with the
     * Joystick Usage is found - this prevents Mice, which use identical descriptors except for the Mouse usage
     * parent node, from being erroneously treated as a joystick
     */
    for (HID_CollectionPath_t *CurrPath = CurrentItem.CollectionPath; CurrPath != NULL; CurrPath = CurrPath->Parent) {
        if ((CurrPath->Usage.Page == USAGE_PAGE_GENERIC_DCTRL) &&
            (CurrPath->Usage.Usage == USAGE_JOYSTICK)) {
            IsRecognized = true;
            break;
        }
    }

    /* If a collection with the joystick usage was not found, indicate that we are not interested in this item */
    if (!IsRecognized)
        return;

    /* Check the attributes of the current joystick item - see if we are interested in it or not;
     * only store BUTTON and GENERIC_DESKTOP_CONTROL items into the Processed HID Report
     * structure to save RAM and ignore the rest
     */
    if ((CurrentItem.Attributes.Usage.Page == USAGE_PAGE_GENERIC_DCTRL)) {

        if (CurrentItem.Attributes.Usage.Usage == USAGE_X) {
            //std::cout << "Found X" << std::endl;
            Joystick.X = CurrentItem;
        }

        if (CurrentItem.Attributes.Usage.Usage == USAGE_Y) {
            //std::cout << "Found Y" << std::endl;
            Joystick.Y = CurrentItem;
        }
    }

    if ((CurrentItem.Attributes.Usage.Page == USAGE_PAGE_BUTTON)) {
        //if (CurrentItem.Attributes.Usage.Usage == 1)
        if (Joystick.btncnt < sizeof Joystick.btnids) {
            //std::cout << "Found Button" << std::endl;
            Joystick.btnids[Joystick.btncnt++] = CurrentItem.BitOffset;
        }
    }
}

bool JoystickMiniport::Poll(USBDriver &Driver) {
    if (Joystick.UsbAddress == 0)
        return false;

    Driver.RequestBulkIn(8, 0x80, Joystick);
    return true;
}

void JoystickMiniport::OnComplete(uint8_t Address, uint8_t *Buffer) {
    if (Address != Joystick.UsbAddress)
        return;

    int bacc = 0;

    for (const auto &item: Joystick.btnids)
        if (item != 0)
            bacc |= GetItemInfo(Buffer, HID_ReportItem(item));

    auto x = GetItemInfo(Buffer, Joystick.X);
    auto y = GetItemInfo(Buffer, Joystick.Y);

    Joystick.bits = 0;
    if (x == Joystick.X.log_min)
        Joystick.bits |= 2;
    if (x == Joystick.X.log_max)
        Joystick.bits |= 1;
    if (y == Joystick.Y.log_min)
        Joystick.bits |= 8;
    if (y == Joystick.Y.log_max)
        Joystick.bits |= 4;
    if (bacc != 0)
        Joystick.bits |= 16;
}

uint8_t JoystickMiniport::GetBits() {
    return Joystick.UsbAddress != 0 ? Joystick.bits : 0;
}

void JoystickMiniport::Reset() {
    IsRecognized = false;
    Joystick.Reset();
}

void JoystickFDO::Reset() {
    btncnt = 0;
    memset(&btnids, 0, sizeof btnids);
    memset(&X, 0, sizeof X);
    memset(&Y, 0, sizeof Y);
    UsbAddress = 0;
    Endpoint = 0;
    Flags.IsLowSpeedDevice = false;
    Flags.SndToggle = false;
    Flags.RcvToggle = false;
}
