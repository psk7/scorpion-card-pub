#ifndef SC_MULTICARD_USB_COROUTINES_H
#define SC_MULTICARD_USB_COROUTINES_H

#include "coroutines.h"

namespace USBH {
    struct usb_coro_allocator {
        static void *operator new(std::size_t size) noexcept;

        static void operator delete(void *ptr, std::size_t size);

        static void register_handle(std::coroutine_handle<> handle);

        static void unregister_handle();
    };

    template<typename T>
    using task = base_task<T, usb_coro_allocator, std::suspend_never>;

    struct delay_res {};

    std::suspend_always operator co_await(delay_res d) noexcept;
}

#endif //SC_MULTICARD_USB_COROUTINES_H
