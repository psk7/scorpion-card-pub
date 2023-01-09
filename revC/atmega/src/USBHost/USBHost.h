#ifndef CH_USBHOST_H
#define CH_USBHOST_H

#include <stdint.h>

#include "StdDescriptors.h"
#include "StdRequestType.h"
#include "usb_coroutines.h"

#define USB_REQUEST_GET_STATUS 0
#define USB_REQUEST_CLEAR_FEATURE 1
#define USB_REQUEST_SET_FEATURE 3
#define USB_REQUEST_SET_ADDRESS 5
#define USB_REQUEST_GET_DESCRIPTOR 6
#define USB_REQUEST_SET_DESCRIPTOR 7
#define USB_REQUEST_GET_CONFIGURATION 8
#define USB_REQUEST_SET_CONFIGURATION 9
#define USB_REQUEST_GET_INTERFACE 10
#define USB_REQUEST_SET_INTERFACE 11
#define USB_REQUEST_SYNC_FRAME 12

#define USB_DEVICE_DESC_TYPE 1
#define USB_CFG_DESC_TYPE 2
#define USB_STR_DESC_TYPE 3
#define USB_IFACE_DESC_TYPE 4
#define USB_EP_DESC_TYPE 5
#define USB_DEVICE_QR_DESC_TYPE 6
#define USB_OSPEED_CFG_DESC_TYPE 7
#define USB_IFACE_PWR_DESC_TYPE 8

typedef struct _USB_DEVICE_DESCRIPTOR_ {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} USB_DEVICE_DESCRIPTOR;

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
