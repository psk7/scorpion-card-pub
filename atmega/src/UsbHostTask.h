#ifndef SC_MULTICARD_USBHOSTTASK_H
#define SC_MULTICARD_USBHOSTTASK_H

#ifdef __cplusplus
extern "C" {
#endif

    void UsbHostTask_Init();
    void UsbHostTask_Run(void *debugdata, bool outdevices, bool outkbd);
    unsigned char UsbHostTask_GetCurrentState();

#ifdef __cplusplus
};
#endif

#endif //SC_MULTICARD_USBHOSTTASK_H
