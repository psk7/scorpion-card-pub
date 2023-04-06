#ifndef CH_USBHOST_H
#define CH_USBHOST_H

#include <stdint.h>

#include "StdDescriptors.h"
#include "StdRequestType.h"
#include "usb_coroutines.h"

#define DRIVERS_TABLE_SIZE 4

extern unsigned char *heap_ptr;

extern bool PrintReports;

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
            bool IsProbed: 1;
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

    class DescriptorParser;

    class USBDriver {
    public:
        virtual task<bool>
        Analyze(USBTarget &Device, uint8_t DeviceClass, const USBH::DescriptorParser &Descriptor) = 0;

        virtual task<void> Probe() { co_return; };

        static task<void> SetConfiguration(uint8_t ConfigurationId);

        static task<bool> Control(USB_Request_Header_t &Request, USBTarget &Target);

        static task<unsigned char *>
        ControlRead(void *buffer, USB_Request_Header_t &Request, uint8_t Size, USBTarget &Target, bool AllowNAK = false);

        task<void>
        SetupControlReadAsync(USB_Request_Header_t &Request, USBTarget &Target);

        static StreamReader<> ReceiveControlAsync(uint8_t Size, uint8_t &ReadedBytes, bool Print);

        static task<void>
        ControlWrite(const USB_Request_Header_t &Command, uint8_t Size, uint8_t *data, USBTarget &Target);

        virtual void Reset() = 0;

        virtual task<bool> Poll() = 0;

        static task<void *> BulkIn(void *buffer, uint8_t Size, USBTarget &Target);
    };

    delay_res Delay(uint8_t ms);

    typedef USBDriver *const DriversTable[DRIVERS_TABLE_SIZE];

    void Init();

    void Init(DriversTable &Drivers);

    bool Run();

    bool IsHostTaskRunning();

    class Alloc {
    private:
        uint16_t Size;
        void *buf;
    public:
        explicit Alloc() : Size(0) { buf = nullptr; }
        explicit Alloc(uint16_t Size) : Size(Size) { buf = heap_ptr; heap_ptr += Size; }

        void* operator*() { return buf; }

        ~Alloc() { release(); }

        void release() {
            if (buf == nullptr)
                return;

            heap_ptr -= Size;
            buf = nullptr;
        }
    };
}

#endif //CH_USBHOST_H
