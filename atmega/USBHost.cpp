#include "USBHost.h"
#include "Link.h"

#ifdef WIN32

#include <iostream>

#else

#include "Timers.h"
#include <string.h>
#include "util/delay.h"

#endif

#include "MAX3421.h"

enum States {
    BusReset,                           // 0    0
    BusReset1,                          // 1    1
    BusReset2,                          // 2    2
    BusReset3,                          // 3    3
    BusReset4,                          // 4    4
    BusReset5,                          // 5    5
    Delay,                              // 6    6
    ControlPending,                     // 7    7
    ReceiveStream,                      // 8    8
    HS_OUT,                             // 9    9
    HS_IN,                              // 10   :
    CheckBus,                           // 11   ;
    ReadDeviceDescriptorPre,            // 12   <
    ReceiveDeviceDescriptorPre,         // 13   =
    ReadDeviceDescriptor,               // 14   >
    ReceiveDeviceDescriptor,            // 15   ?
    SetDeviceAddress,                   // 16   @
    ReadConfigurationDescriptorPre,     // 17   A
    ReceiveConfigurationDescriptorPre,  // 18   B
    ReadConfigurationDescriptor,        // 19   C
    ReceiveConfigurationDescriptor,     // 20   D
    ParseConfigurationDescriptor,       // 21   E
    DriverPending,                      // 22   F
    DriverComplete,                     // 23   G
    DriverPolling,                      // 24   H
    DriverControlAckOut,                // 25   I
    DriverCompleteBulk,                 // 26   J
    SendStream,                         // 27   K
    ReadDeviceDescriptor2,              // 28   L
    ReadDeviceDescriptor3               // 29   M
};

States CurrentState = BusReset;
States NextState = BusReset;
States NextNextState = BusReset;
States ControlNextState = BusReset;
uint8_t PendingDelay;

uint8_t buffer[USB_BUFFER_SIZE];
uint8_t *buffer_ptr = &buffer[0];
int bytes_to_transfer;
uint8_t usb_hxfr;
uint8_t real_descriptor_size;
uint8_t usb_dev_class;
uint8_t usb_next_address = 1;
USBDriver *ActiveDriver;
USBTarget *pending_target;
uint8_t pending_driver_id;
Result usb_last_result;
PortInfo RootPortInfo;
PortInfo *pPortInfo = &RootPortInfo;
uint8_t PollDriverIdx = 0;

DriversTable *Drivers;

union _Flags {
    struct {
        bool bus_state: 1;
        bool DisableDeviceDiscovery: 1;
    };
    uint8_t Val;
} Flags{};

#ifdef WIN32

void OutError(Result Result) {

    switch (Result) {

        case hrSUCCESS:
            std::cout << "hrSUCCESS";
            break;
        case hrBUSY:
            std::cout << "hrBUSY";
            break;
        case hrNAK:
            std::cout << "hrNAK";
            break;
        case hrSTALL:
            std::cout << "hrSTALL";
            break;
        case hrTOGERR:
            std::cout << "hrTOGERR";
            break;
        case hrKERR:
            std::cout << "hrKERR";
            break;
        case hrJERR:
            std::cout << "hrJERR";
            break;
        case hrTIMEOUT:
            std::cout << "hrTIMEOUT";
            break;

        default:
            std::cout << (int) Result;
            break;
    }
}

#endif

void UniDelay(uint8_t Delay) {
#ifdef WIN32
    Sleep(Delay);
#else
    for (uint8_t i = 0; i < Delay; i++)
        _delay_ms(1);
#endif
}

Result ReadResult() {
    return (Result) (ReadUSB(HRSL) & 0xf);
}

inline void WriteHXFR(uint8_t value) {
    while (ReadResult() == Result::hrBUSY);

#ifdef WIN32
    Sleep(1);
#else
    _delay_us(10);
#endif

    WriteUSB(HXFR, value);

#ifdef WIN32
    Sleep(1);
#else
    _delay_us(10);
#endif
}

inline void WaitComplete() {
    while ((ReadUSB(HIRQ) & HXFRDNIRQ) == 0);
    usb_last_result = ReadResult();
    WriteUSB(HIRQ, HXFRDNIRQ);
}

inline void BlockingHXFR(uint8_t value) {
    WriteHXFR(value);
    WaitComplete();
}

inline void USBXFHR(uint8_t cmd) {
    WriteHXFR(cmd);
    while ((ReadUSB(HIRQ) & HXFRDNIRQ) == 0);
    usb_last_result = ReadResult();
}

void SaveToggle(USBTarget &Obj) {
    uint8_t hrsl = ReadUSB(HRSL);

    Obj.Flags.RcvToggle = hrsl & RCVTOGRD;
    Obj.Flags.SndToggle = hrsl & SNDTOGRD;
}

void RestoreToggle(const USBTarget &Obj) {
    uint8_t rcv = Obj.Flags.RcvToggle ? RCVTOG1 : RCVTOG0;
    uint8_t snd = Obj.Flags.SndToggle ? SNDTOG1 : SNDTOG0;

    WriteUSB(HCTL, rcv | snd);
}

void SetupTarget(const USBTarget &Obj) {
    auto v = DPPULLDN | DMPULLDN | HOST | SOFKAENAB;

    if (Obj.Flags.IsLowSpeedDevice)
        v |= LOWSPEED;

    if (!RootPortInfo.LowSpeed && Obj.Flags.IsLowSpeedDevice)
        v |= HUBPRE;

    WriteUSB(MODE, v);
    WriteUSB(PERADDR, Obj.UsbAddress);
}

void WriteUSBControl(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7,
                     uint8_t b8, States NextStateAfterControl) {
    ControlNextState = NextStateAfterControl;

    WriteUSBControl(b1, b2, b3, b4, b5, b6, b7, b8);

    WriteHXFR(0x10);     // SETUP
    CurrentState = States::ControlPending;
}

void USBHost_Init(DriversTable &Drivers) {
    ::Drivers = &Drivers;
    CurrentState = BusReset;

    for (auto &item: Drivers)
        item->Reset();
}

void SetupDelay(uint8_t ms, States StateAfterDelay) {
#ifdef WIN32
    PendingDelay = ms;
#else
    Timers_SetupUSBDelay(ms);
#endif

    CurrentState = States::Delay;
    NextState = StateAfterDelay;
}

void SetupReceiveStream(uint8_t size, uint8_t xhfr, States Next, States NextNext) {
    bytes_to_transfer = size;
    usb_hxfr = xhfr; // IN
    WriteHXFR(usb_hxfr);

    buffer_ptr = &buffer[0];
    NextState = Next;
    NextNextState = NextNext;
    CurrentState = States::ReceiveStream;
}

void SetupSendStream(uint8_t size, uint8_t xhfr, States Next, States NextNext) {
    bytes_to_transfer = size;
    usb_hxfr = xhfr; // IN

    buffer_ptr = &buffer[0];

    NextState = Next;
    NextNextState = NextNext;
    CurrentState = States::SendStream;
}

void SetupSendStream(uint8_t size, uint8_t xhfr, uint8_t *data, States Next, States NextNext) {
    SetupSendStream(size, xhfr, Next, NextNext);

    memcpy(buffer_ptr, data, size);
}

void USBHost_FillDebug(uint8_t *ptr) {
    if (ptr == nullptr)
        return;

    ptr[0] = CurrentState;
    ptr[1] = NextState;
    ptr[2] = NextNextState;
    ptr[3] = usb_last_result;
    ptr[4] = pPortInfo->Address;
    ptr[5] = pPortInfo->Flags;
    ptr[6] = PollDriverIdx;
    ptr[7] = Flags.Val;
    ptr[8] = bytes_to_transfer;
    ptr[9] = real_descriptor_size;
    ptr[10] = usb_hxfr;
    memcpy(&ptr[16], &buffer[0], 48);
}

void USBHost_Run() {
/*
    std::cout << "CS: " << std::dec << CurrentState << " last_result = ";
    OutError(usb_last_result);
    std::cout << std::endl;
*/

    if (ReadResult() == Result::hrBUSY)
        return;

    switch (CurrentState) {

        case States::BusReset:
            RootPortInfo.Flags = 0;
            RootPortInfo.IsRootPort = true;
            RootPortInfo.Address = 0;
            PollDriverIdx = 0;

            for (const auto &item: *Drivers)
                item->Reset();

            Flags.Val = 0;
            usb_next_address = 1;
            usb_dev_class = 0;
            CurrentState = BusReset1;
            ActiveDriver = nullptr;
            pending_target = nullptr;
            pending_driver_id = 0;

            Flags.bus_state = false;
            Flags.DisableDeviceDiscovery = false;

#ifdef WIN32
            std::cout << "BusReset                                                   " << std::endl;
#endif
            pPortInfo = &RootPortInfo;
            break;

        case States::BusReset1:
            WriteUSB(PINCTL, FDUPSPI);    // Enable full duplex
            WriteUSB(USBCTL, CHIPRES);    // Set chip reset
            SetupDelay(200, States::BusReset2);
            break;

        case States::BusReset2:
            WriteUSB(USBCTL, 0);  // Reset chip reset
            SetupDelay(200, States::BusReset3);
            break;

        case States::Delay:
#ifdef WIN32
            UniDelay(PendingDelay);
#else
            if (!Timers_CheckUSBDelay())
                break;
#endif

            CurrentState = NextState;
            break;

        case States::ControlPending:
            if (ReadUSB(HIRQ) & HXFRDNIRQ) {
                usb_last_result = ReadResult();

                CurrentState = ControlNextState;
                WriteUSB(HIRQ, HXFRDNIRQ);
            }
            break;

        case States::BusReset3:;
            WriteUSB(MODE, DPPULLDN | DMPULLDN | HOST);
            WriteUSB(HCTL, BUSRST);
            CurrentState = States::BusReset4;
            break;

        case States::BusReset4:
            if (!(ReadUSB(HCTL) & BUSRST))
                SetupDelay(100, States::BusReset5);
            break;

        case States::BusReset5:
            //WriteUSB(MODE, DPPULLDN | DMPULLDN | HOST);
            SetupDelay(100, States::CheckBus);
            break;

        case States::HS_OUT:
            if (usb_last_result == hrSUCCESS)
                USBXFHR(0xa0);

            CurrentState = NextState;
            break;

        case States::HS_IN:
            if (usb_last_result == hrSUCCESS)
                USBXFHR(0x80);

            CurrentState = NextState;
            break;

        case States::ReceiveStream: {
            if ((ReadUSB(HIRQ) & HXFRDNIRQ) == 0)
                break;

            auto hrsl = ReadResult();
            usb_last_result = hrsl;
            WriteUSB(HIRQ, HXFRDNIRQ);

            if (hrsl == Result::hrTOGERR) {
                WriteHXFR(usb_hxfr);
                break;
            }

            if (hrsl != hrSUCCESS) {
                CurrentState = NextState;
                NextState = NextNextState;

#ifdef WIN32
                if (hrsl != hrNAK) {
                    std::cout << "Receive stream exit due ";
                    OutError(hrsl);
                    std::cout << std::endl;
                }
#endif

                break;
            }

            uint16_t already_readed = buffer_ptr - &buffer[0];

            auto rcvbc = (int) ReadUSB(RCBVC);

            if (already_readed + rcvbc > sizeof buffer) {
                CurrentState = BusReset;    // Buffer overflow
                break;
            }

            ReadUSB(RCVFIFO, rcvbc, buffer_ptr);
            WriteUSB(HIRQ, RCVDAVIRQ);

            bytes_to_transfer -= rcvbc;
            buffer_ptr += rcvbc;

            if (bytes_to_transfer <= 0) {
                CurrentState = NextState;
                NextState = NextNextState;
            } else
                WriteHXFR(usb_hxfr);
            break;
        }

        case States::SendStream: {
            //if ((ReadUSB(HIRQ) & SNDBAVIRQ) == 0)
            //    break;

            WriteUSB(SNDFIFO, bytes_to_transfer, buffer_ptr);

            WriteUSB(SNDBC, bytes_to_transfer);

            BlockingHXFR(usb_hxfr);

            bytes_to_transfer = 0;

            if (usb_last_result == Result::hrNAK) {
                CurrentState = NextNextState;
            } else {
                CurrentState = NextState;
                NextState = NextNextState;
            }
            break;
        }

        case States::CheckBus: { /* 11 */
            WriteUSB(PERADDR, 0);

            if ((ReadUSB(HIRQ) & CONDETIRQ) != 0)
                WriteUSB(HIRQ, CONDETIRQ);

            //WriteUSB(HCTL, SAMPLEBUS);
            //while ((ReadUSB(HCTL) & SAMPLEBUS) != 0);

            auto hrsl = ReadUSB(HRSL);
            auto state = (hrsl & (JSTATUS | KSTATUS)) != 0;

            if (Flags.bus_state != state) {
                Flags.bus_state = state;

                if (state != 0) {
                    // Device Attached
                    if (hrsl & KSTATUS) { // Low speed device attached
                        WriteUSB(MODE, DPPULLDN | DMPULLDN | HOST | SOFKAENAB | LOWSPEED);
                        RootPortInfo.LowSpeed = true;
#ifdef WIN32
                        std::cout << "Connected LS                                         " << std::endl;
#endif
                    } else {
                        WriteUSB(MODE, DPPULLDN | DMPULLDN | HOST | SOFKAENAB);
                        RootPortInfo.LowSpeed = false;
#ifdef WIN32
                        std::cout << "Connected FS                                         " << std::endl;
#endif
                    }

                    SetupDelay(250, States::ReadDeviceDescriptorPre);
                    break;
                } else {
                    WriteUSB(MODE, DPPULLDN | DMPULLDN | HOST);

#ifdef WIN32
                    std::cout << "Disconnected                                            " << std::endl;
#endif

                    SetupDelay(250, States::CheckBus);
                    break;
                }
            }

            CurrentState = (state != 0) ? States::ReadDeviceDescriptorPre : States::CheckBus;
            break;
        }

        case States::ReadDeviceDescriptorPre: /* 12 */ {
            if (Flags.DisableDeviceDiscovery) {
                CurrentState = States::DriverPolling;
                break;
            }

            auto v = DPPULLDN | DMPULLDN | HOST | SOFKAENAB;

            if (pPortInfo->LowSpeed)
                v |= LOWSPEED;

            if (!RootPortInfo.LowSpeed && pPortInfo->LowSpeed)
                v |= HUBPRE;

            WriteUSB(MODE, v);

            WriteUSB(PERADDR, 0);

            // GET_DESCRIPTOR (Device)
            WriteUSBControl(0x80, 0x06, 0x00, 1, 0x00, 0x00, 8, 0x00, States::ReceiveDeviceDescriptorPre);
            break;
        }

        case States::ReceiveDeviceDescriptorPre: /* 13 */ {
            if (usb_last_result == Result::hrJERR) {
                SetupDelay(25, States::ReadDeviceDescriptorPre);
                break;
            }

            if (usb_last_result != hrSUCCESS) {
                // No unconfigured devices
                CurrentState = States::DriverPolling;
                break;
            }

            WriteUSB(HCTL, RCVTOG1);

            SetupReceiveStream(8, 0, States::ReadDeviceDescriptor, States::ReadDeviceDescriptor);
            break;
        }

        case States::ReadDeviceDescriptor: /* 14 */
            if (usb_last_result == Result::hrJERR) {
                CurrentState = States::ReadDeviceDescriptorPre;
                break;
            }

            real_descriptor_size = buffer[0];

            if (pPortInfo->IsRootPort) {
                WriteUSB(HCTL, BUSRST);
                CurrentState = States::ReadDeviceDescriptor2;
            } else
                SetupDelay(100, States::ReadDeviceDescriptor3);

            break;

        case States::ReadDeviceDescriptor2: /* 28 */
            if (!(ReadUSB(HCTL) & BUSRST))
                SetupDelay(100, States::ReadDeviceDescriptor3);
            break;

        case States::ReadDeviceDescriptor3: /* 29 */
            // GET_DESCRIPTOR (Device)
            WriteUSBControl(0x80, 0x06, 0x00, 1, 0x00, 0x00, real_descriptor_size, 0x00,
                            States::ReceiveDeviceDescriptor);
            break;

        case States::ReceiveDeviceDescriptor: /* 15 */{
            if (usb_last_result != Result::hrSUCCESS) {
                CurrentState = States::DriverPolling;
                break;
            }

            WriteUSB(HCTL, RCVTOG1);

            SetupReceiveStream(real_descriptor_size, 0, States::HS_OUT, States::SetDeviceAddress); // IN
            break;
        }

        case States::SetDeviceAddress: { /* 16 */
            auto dd = (USB_StdDescriptor_Device_t *) buffer;
            usb_dev_class = dd->bDeviceClass;

            auto addr = usb_next_address++;

            WriteUSBControl(0x0, 0x05, addr, 0x00, 0x00, 0x00, 0x00, 0x00);

            USBXFHR(0x10);
            WriteUSB(HIRQ, HXFRDNIRQ);

            UniDelay(10);

            USBXFHR(0x80);
            WriteUSB(HIRQ, HXFRDNIRQ);

            UniDelay(10);

            WriteUSB(PERADDR, addr);

            pPortInfo->Address = addr;
            Flags.DisableDeviceDiscovery = true;

#ifdef WIN32
            std::cout << "New device found @" << (int) addr
                      << "                                                                         " << std::endl;
#endif

            CurrentState = States::ReadConfigurationDescriptorPre;
            break;
        }

        case States::ReadConfigurationDescriptorPre: /* 17 */
            // GET_DESCRIPTOR (Configuration)
            WriteUSBControl(0x80, 0x06, 0x00, 2, 0x00, 0x00, 8, 0x00, States::ReceiveConfigurationDescriptorPre);
            break;

        case States::ReceiveConfigurationDescriptorPre: /* 18 */
            WriteUSB(HCTL, RCVTOG1);

            SetupReceiveStream(8, 0, States::HS_OUT, States::ReadConfigurationDescriptor); // IN
            break;

        case States::ReadConfigurationDescriptor: {
            auto cd = (USB_StdDescriptor_Configuration_Header_t *) buffer;
            auto size = cd->wTotalLength;

            if (size > sizeof buffer) {
                CurrentState = BusReset;
                break;
            }

            real_descriptor_size = size;

            // GET_DESCRIPTOR (Configuration)
            WriteUSBControl(0x80, 0x06, 0x00, 2, 0x00, 0x00, real_descriptor_size, 0x00,
                            States::ReceiveConfigurationDescriptor);
            break;
        }

        case States::ReceiveConfigurationDescriptor: {
            WriteUSB(HCTL, RCVTOG1);

            SetupReceiveStream(real_descriptor_size, 0, States::HS_OUT, States::ParseConfigurationDescriptor); // IN
            break;
        }

        case States::ParseConfigurationDescriptor: {
            uint8_t *ptr = &buffer[0];

            Flags.DisableDeviceDiscovery = true;

            auto ds = real_descriptor_size;

            USB_StdDescriptor_Configuration_Header_t *conf_descriptor = nullptr;
            USB_StdDescriptor_Interface_t *interf_descriptor = nullptr;

            while ((ptr - &buffer[0]) < ds) {
                //std::cout << "Descriptor type " << (int) ptr[1] << " found" << std::endl;

                if (ptr[1] == 2) {
                    conf_descriptor = (USB_StdDescriptor_Configuration_Header_t *) ptr;
                } else if (ptr[1] == 4) {
                    interf_descriptor = (USB_StdDescriptor_Interface_t *) ptr;

                    auto addr = ReadUSB(PERADDR);
                    for (const auto &item: *Drivers)
                        if (item != nullptr)
                            if (item->Analyze(addr, pPortInfo->LowSpeed, usb_dev_class,
                                              conf_descriptor, interf_descriptor))
                                return;

                    // Device is not recognized by any driver
                    usb_dev_class = 0;
                    if (!pPortInfo->IsRootPort) {
                        pPortInfo->Configured = true;       // To leave device alone
                        pPortInfo->Configuring = false;
                    }
                    break;
                }

                ptr += ptr[0];
            }

            CurrentState = States::CheckBus;
            break;
        }

        case States::DriverPending:
            if (usb_last_result == Result::hrSUCCESS) {
                if ((ReadUSB(HIRQ) & HXFRDNIRQ) == 0)
                    break;

                WriteUSB(HIRQ, HXFRDNIRQ);      // <-- There is NO break after WriteUSB. It's OK.
            }

        case States::DriverComplete: {
            if (pending_target != nullptr) {
                SaveToggle(*pending_target);
                pending_target = nullptr;
            }

            auto ptr = &buffer[0];
            usb_last_result = ReadResult();

            if (usb_last_result == Result::hrNAK)
                ptr = nullptr;

            auto d = ActiveDriver;
            ActiveDriver = nullptr;
            d->OnComplete(ptr, pending_driver_id, *pPortInfo);
            break;
        }

        case States::DriverCompleteBulk: {
            if (pending_target != nullptr) {
                SaveToggle(*pending_target);
                pending_target = nullptr;
            }

            auto ptr = &buffer[0];
            usb_last_result = ReadResult();

            if (usb_last_result == Result::hrNAK)
                ptr = nullptr;

            auto d = ActiveDriver;
            ActiveDriver = nullptr;
            d->OnBulkComplete(ptr, pending_driver_id, ReadUSB(PERADDR));
            break;
        }

        case States::DriverPolling: {
            auto d = (*Drivers)[PollDriverIdx];

            if (++PollDriverIdx == DRIVERS_TABLE_SIZE)
                PollDriverIdx = 0;

            if (d != nullptr && d->Poll())
                return;

            // No driver command issued
            CurrentState = States::CheckBus;
            break;
        }

        case States::DriverControlAckOut:
            WriteHXFR(0xa0);
            CurrentState = NextState;
            break;

        default:
            CurrentState = BusReset;
    }

}

uint8_t USBHost_GetCurrentState() {
    return CurrentState;
}

void USBDriver::RequestControl(uint8_t *Command, uint8_t Id, USBTarget &Target) {
    SetupTarget(Target);
    RequestControl(Command, Id);
}

void USBDriver::RequestControl(uint8_t *Command, uint8_t Id) {
    ActiveDriver = this;
    pending_driver_id = Id;
    pending_target = nullptr;

    WriteUSBControl(Command[0], Command[1], Command[2], Command[3], Command[4], Command[5], Command[6], Command[7]);

    BlockingHXFR(0x10);
    BlockingHXFR(0x80);

    CurrentState = States::DriverComplete;
}

void USBDriver::RequestControlRead(uint8_t (&Command)[8], uint8_t Size, uint8_t Id, USBTarget &Target) {
    SetupTarget(Target);
    RequestControlRead(Command, Size, Id);
}

void USBDriver::RequestControlRead(uint8_t (&Command)[8], uint8_t Size, uint8_t Id) {
    ActiveDriver = this;
    pending_driver_id = Id;
    WriteUSBControl(Command);

    WriteHXFR(0x10);
    UniDelay(1);
    WaitComplete();

    WriteUSB(HCTL, RCVTOG1);
    SetupReceiveStream(Size, 0, States::HS_OUT, States::DriverPending);
}

void USBDriver::RequestControlWrite(uint8_t (&Command)[8], uint8_t Size, uint8_t *data, uint8_t Id, USBTarget &Target) {
    SetupTarget(Target);
    RequestControlWrite(Command, Size, data, Id);
}

void USBDriver::RequestControlWrite(uint8_t (&Command)[8], uint8_t Size, uint8_t *data, uint8_t Id) {
    ActiveDriver = this;
    pending_driver_id = Id;
    WriteUSBControl(Command);

    WriteHXFR(0x10);
    //UniDelay(1);
    WaitComplete();

    WriteUSB(HCTL, RCVTOG1);
    SetupSendStream(Size, 0x20, data, States::HS_IN, States::DriverPending);
}

void USBDriver::SetConfiguration(uint8_t ConfigurationId) {
    WriteUSBControl(0x0, 0x09, ConfigurationId, 0x00, 0x00, 0x00, 0x00, 0x00); // SET_CONFIGURATION
    USBXFHR(0x10);
    WriteUSB(HIRQ, HXFRDNIRQ);
    //auto res = ReadResult();

    USBXFHR(0x80);
    WriteUSB(HIRQ, HXFRDNIRQ);
    //res = ReadResult();

    pPortInfo->Configured = true;
    pPortInfo->Configuring = false;

    CurrentState = States::DriverPolling;
}

void USBDriver::CompleteEnumeration() {
    CurrentState = States::CheckBus;
}

void USBDriver::RequestBulkIn(int8_t Size, uint8_t Id, USBTarget &Target) {
    SetupTarget(Target);
    RestoreToggle(Target);
    pending_target = &Target;
    ActiveDriver = this;
    pending_driver_id = Id;
    SetupReceiveStream(Size, 0 | Target.Endpoint, States::DriverCompleteBulk, States::CheckBus);
}

void USBDriver::RequestBulkOut(uint8_t *data, int8_t Size, uint8_t Id, USBTarget &Target) {
    SetupTarget(Target);
    RestoreToggle(Target);
    pending_target = &Target;
    ActiveDriver = this;
    pending_driver_id = Id;
    SetupSendStream(Size, 0x20 | Target.Endpoint, data, States::DriverCompleteBulk, States::CheckBus);
}

void USBDriver::CompletePoll() {
    CurrentState = States::CheckBus;
}

void USBDriver::ResetUsbStack() {
    CurrentState = States::BusReset;
}

void USBDriver::RequestDelay(uint8_t MS, uint8_t Id) {
    pending_driver_id = Id;
    ActiveDriver = this;
    SetupDelay(MS, States::DriverComplete);
}

void USBDriver::SetPortInfo(PortInfo *PortInfo) {
    pPortInfo = PortInfo;
}

void USBDriver::EnableNewDeviceDiscovery() {
    Flags.DisableDeviceDiscovery = false;
}

void USBDriver::DisableNewDeviceDiscovery() {
    Flags.DisableDeviceDiscovery = true;
}
