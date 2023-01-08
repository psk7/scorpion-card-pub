#include "HIDJoystickDriver.h"
#include "HIDClassCommon.h"
#include "usblib.h"
#include "USBHost.h"
#include "HIDReport.h"
#include "Joystick.h"

using namespace USBH;

task<bool>
HIDJoystickDriver::Analyze(USBTarget &Device, uint8_t DeviceClass, USB_StdDescriptor_Configuration_Header_t *Configuration,
                           USB_StdDescriptor_Interface_t *Interface) {
    if (UsbAddress != 0)
        co_return false;

    if (DeviceClass == 0)
        DeviceClass = Interface->bInterfaceClass;

    if (DeviceClass != HID_CSCP_HIDClass)
        co_return false;

    auto *ptr = (uint8_t *) Interface;

    while (true) {
        if (ptr[1] == 33)
            break;

        ptr += ptr[0];
    }

    auto hid = (USBH::_HID_DESCRIPTOR *) ptr;

    auto reportDescriptorSize = hid->DescriptorList[0].wReportLength;

    if (reportDescriptorSize > USB_BUFFER_SIZE)
        co_return false;

    auto confId = Configuration->bConfigurationValue;

    //uint8_t c[] = {0x81, 0x06, 0x00, joystickDriver->DescriptorList[0].bReportType, 0x00, 0x00, ReportDescriptorSize, 0x00};
    auto req = USB_Request_Header_t{
            .bmRequestType = 0x81,
            .bRequest      = USB_REQUEST_GET_DESCRIPTOR,
            .wValue        = static_cast<uint16_t>(hid->DescriptorList[0].bReportType) << 8,
            .wIndex        = 0,
            .wLength       = reportDescriptorSize
    };

    auto reportDescriptor = co_await ControlRead(req, reportDescriptorSize, Device);

    if (reportDescriptor == nullptr)
        co_return false;

    HID_ReportInfo_t info;
    ParseDescriptor(reportDescriptor, reportDescriptorSize, &info);
    ReportSize = (info.LargestReportSizeBits + 7) >> 3;

    if (!IsRecognized)
        co_return false;

    UsbAddress = Device.UsbAddress;
    Endpoint = 1;
    Flags.IsLowSpeedDevice = Device.Flags.IsLowSpeedDevice;

    co_await SetConfiguration(confId);

    co_return true;
}

void HIDJoystickDriver::OnItem(const HID_ReportItem_t &CurrentItem) {
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
            X = CurrentItem;
        }

        if (CurrentItem.Attributes.Usage.Usage == USAGE_Y) {
            //std::cout << "Found Y" << std::endl;
            Y = CurrentItem;
        }
    }

    if ((CurrentItem.Attributes.Usage.Page == USAGE_PAGE_BUTTON)) {
        //if (CurrentItem.Attributes.Usage.Usage == 1)
        if (btncnt < sizeof btnids) {
            //std::cout << "Found Button" << std::endl;
            btnids[btncnt++] = CurrentItem.BitOffset;
        }
    }
}

task<bool> HIDJoystickDriver::Poll() {
    if (UsbAddress == 0)
        co_return true;

    auto buf = co_await BulkIn(ReportSize, *this);

    if (buf == nullptr)
        co_return true;

    int bit = 1 << 4;

    bits = 0;

    for (const auto &item: btnids)
        if (item != 0) {
            if (GetItemInfo(buf, HID_ReportItem(item)))
                bits |= bit;

            bit <<= 1;
        }

    auto x = GetItemInfo(buf, X);
    auto y = GetItemInfo(buf, Y);

    if (x == X.log_min)
        bits |= JOYSTICK_LEFT;
    if (x == X.log_max)
        bits |= JOYSTICK_RIGHT;
    if (y == Y.log_min)
        bits |= JOYSTICK_UP;
    if (y == Y.log_max)
        bits |= JOYSTICK_DOWN;

    co_return true;
}

void HIDJoystickDriver::Reset() {
    IsRecognized = false;
    bits = {};
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

uint16_t HIDJoystickDriver::GetBits() {
    return UsbAddress != 0 ? bits : 0;
}

