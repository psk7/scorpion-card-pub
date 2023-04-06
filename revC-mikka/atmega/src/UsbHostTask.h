#ifndef SC_MULTICARD_USBHOSTTASK_H
#define SC_MULTICARD_USBHOSTTASK_H

#include "usb_coroutines.h"

namespace USBT {
    struct delay_res {};

    std::suspend_always operator co_await(delay_res d) noexcept;

    void Init();

    void Run();
}

#endif //SC_MULTICARD_USBHOSTTASK_H
