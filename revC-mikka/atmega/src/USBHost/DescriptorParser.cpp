#include "DescriptorParser.h"

bool USBH::DescriptorParser::NextInterface() {
    Interface = nullptr;
    EndPointIn = nullptr;
    EndPointOut = nullptr;
    HidDescriptor = nullptr;

    while (ptr < end) {
        auto type = ptr[1];

        if (type == 4 && Interface != nullptr)
            return true;

        if (type == 4)
            Interface = (USB_StdDescriptor_Interface_t *) ptr;
        else if (type == 5) {
            auto EndPoint = (USB_StdDescriptor_Endpoint_t *) ptr;
            if (EndPoint->bEndpointAddress & 0x80)
                EndPointIn = EndPoint;
            else
                EndPointOut = EndPoint;
        } else if (type == 33)
            HidDescriptor = (USBH::_HID_DESCRIPTOR *) ptr;

        ptr += ptr[0];
    }

    return Interface != nullptr;
}
