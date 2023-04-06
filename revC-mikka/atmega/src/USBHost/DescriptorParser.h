#ifndef SC_MULTICARD_DESCRIPTORPARSER_H
#define SC_MULTICARD_DESCRIPTORPARSER_H

#include "StdDescriptors.h"
#include "USBHost.h"

namespace USBH {
    class DescriptorParser {
    public:
        const USB_StdDescriptor_Configuration_Header_t *Configuration = nullptr;

    private:
        uint8_t *ptr;
        uint8_t *end;

    public:
        USB_StdDescriptor_Interface_t *Interface = nullptr;
        USB_StdDescriptor_Endpoint_t *EndPointIn = nullptr;
        USB_StdDescriptor_Endpoint_t *EndPointOut = nullptr;
        struct _HID_DESCRIPTOR *HidDescriptor = nullptr;

    public:
        explicit DescriptorParser(void *ConfigurationDescriptor) :
                DescriptorParser((USB_StdDescriptor_Configuration_Header_t *) ConfigurationDescriptor) {}

        explicit DescriptorParser(USB_StdDescriptor_Configuration_Header_t *ConfigurationDescriptor) :
                Configuration(ConfigurationDescriptor),
                ptr((uint8_t *) ConfigurationDescriptor),
                end(ptr + ConfigurationDescriptor->wTotalLength) {}

        bool NextInterface();
    };
}

#endif //SC_MULTICARD_DESCRIPTORPARSER_H
