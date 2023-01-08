#ifndef CH_USBHOST_H
#define CH_USBHOST_H

#include <stdint.h>

#include "StdDescriptors.h"
#include "StdRequestType.h"
#include "usb_coroutines.h"

#define USB_BUFFER_SIZE 128
#define DRIVERS_TABLE_SIZE 3

namespace USBH {
    struct _HID_DESCRIPTOR {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint16_t bcdHID;
        uint8_t bCountry;
        uint8_t bNumDescriptors;

        struct _HID_DESCRIPTOR_DESC_LIST {
            uint8_t bReportType;
            uint16_t wReportLength;
        };

        _HID_DESCRIPTOR_DESC_LIST DescriptorList[1];
    };

    class USBTarget {
    public:
        uint8_t UsbAddress{};
        uint8_t Endpoint{};

        struct {
            bool RcvToggle: 1;
            bool SndToggle: 1;
            bool IsLowSpeedDevice: 1;
        } Flags{};
    };

    class PortInfo : public USBTarget {
    public:
        virtual task<void> ResetPort() = 0;
    };

    class RootPortInfo : public PortInfo {
    public:
        task<void> ResetPort() override;
    };

    task<bool> InitializeNewDevice(PortInfo &PortInfo);

    class USBDriver {
    public:
        virtual task<bool>
        Analyze(USBTarget &Device, uint8_t DeviceClass, USB_StdDescriptor_Configuration_Header_t *Configuration,
                USB_StdDescriptor_Interface_t *Interface) = 0;

        static task<void> SetConfiguration(uint8_t ConfigurationId);

        static task<bool> Control(USB_Request_Header_t &Request, USBTarget &Target);

        static task<unsigned char *>
        ControlRead(USB_Request_Header_t &Request, uint8_t Size, USBTarget &Target, bool AllowNAK = false);

        static task<void>
        ControlWrite(const USB_Request_Header_t &Command, uint8_t Size, uint8_t *data, USBTarget &Target);

        virtual void Reset() = 0;

        virtual task<bool> Poll() = 0;

        static task<unsigned char *> BulkIn(int8_t Size, USBTarget &Target);
    };

    delay_res Delay(uint8_t ms);

    typedef USBDriver *const DriversTable[DRIVERS_TABLE_SIZE];

    void Init();

    void Init(DriversTable &Drivers);

    void Run();
}

#endif //CH_USBHOST_H
