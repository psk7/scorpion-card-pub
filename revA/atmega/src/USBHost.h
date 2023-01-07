#ifndef CH_USBHOST_H
#define CH_USBHOST_H

#ifdef WIN32

#include <Windows.h>
#include <cstdint>

#else
#include <stdint.h>
#endif

#include "StdDescriptors.h"

#define USB_BUFFER_SIZE 128
#define DRIVERS_TABLE_SIZE 4

#pragma pack(push)

#pragma pack(1)
typedef struct _HID_DESCRIPTOR {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountry;
    uint8_t bNumDescriptors;

#pragma pack(1)
    struct _HID_DESCRIPTOR_DESC_LIST {
        uint8_t bReportType;
        uint16_t wReportLength;
    };

    _HID_DESCRIPTOR_DESC_LIST DescriptorList[1];
} HID_DESCRIPTOR, *PHID_DESCRIPTOR;

class HUBDriver;

class PortInfo {
public:
    uint8_t Address;
    union {
        uint8_t Flags;
        struct {
            bool IsRootPort: 1;
            bool LowSpeed: 1;
            bool Configured: 1;
            bool Configuring: 1;
            bool Connected: 1;
        };
    };
};

class USBTarget {
public:
    uint8_t UsbAddress = 0;
    uint8_t Endpoint = 0;

    struct {
        bool RcvToggle: 1;
        bool SndToggle: 1;
        bool IsLowSpeedDevice: 1;
    } Flags;
};

class USBDriver {
public:
    virtual bool
    Analyze(uint8_t Address, bool IsLowSpeed, uint8_t DeviceClass,
            USB_StdDescriptor_Configuration_Header_t *Configuration, USB_StdDescriptor_Interface_t *Interface) = 0;

    void SetConfiguration(uint8_t ConfigurationId);

    void RequestControl(uint8_t *Command, uint8_t Id);

    void RequestControl(uint8_t *Command, uint8_t Id, USBTarget &Target);

    void RequestControlRead(uint8_t (&Command)[8], uint8_t Size, uint8_t Id);

    void RequestControlRead(uint8_t (&Command)[8], uint8_t Size, uint8_t Id, USBTarget &Target);

    void RequestControlWrite(uint8_t (&Command)[8], uint8_t Size, uint8_t *data, uint8_t Id);

    void RequestControlWrite(uint8_t (&Command)[8], uint8_t Size, uint8_t *data, uint8_t Id, USBTarget &Target);

    void RequestDelay(uint8_t MS, uint8_t Id);

    static void SetPortInfo(PortInfo *PortInfo);

    static void CompleteEnumeration();

    static void CompletePoll();

    static void ResetUsbStack();

    virtual void Reset() = 0;

    virtual void OnComplete(uint8_t *Buffer, uint8_t Id, PortInfo &Port) = 0;

    virtual void OnBulkComplete(uint8_t *Buffer, uint8_t Id, uint8_t Address) = 0;

    virtual bool Poll() = 0;

    void RequestBulkIn(int8_t Size, uint8_t Id, USBTarget &Target);

    void RequestBulkOut(uint8_t *data, int8_t Size, uint8_t Id, USBTarget &Target);

    static void EnableNewDeviceDiscovery();

    static void DisableNewDeviceDiscovery();
};

typedef USBDriver* const DriversTable[DRIVERS_TABLE_SIZE];

void USBHost_Init(DriversTable &Drivers);

void USBHost_Run();

uint8_t USBHost_GetCurrentState();

void USBHost_FillDebug(uint8_t *ptr);

#pragma pack(pop)
#endif //CH_USBHOST_H
