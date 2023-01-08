#ifndef SC_MULTICARD_USBHOSTTASK_H
#define SC_MULTICARD_USBHOSTTASK_H

#include "coroutines.h"

namespace USBT {
    struct coro_allocator {
        static void *operator new(std::size_t size) noexcept;

        static void operator delete(void *ptr, std::size_t size);

        static void register_handle(std::coroutine_handle<> handle);

        static void unregister_handle();
    };

    template<typename T>
    using task = base_task<T, coro_allocator, std::suspend_always>;

    struct delay_res {};

    std::suspend_always operator co_await(delay_res d) noexcept;
    
    void Init();

    void Run();

    task<void> Run_Coro();
}

#endif //SC_MULTICARD_USBHOSTTASK_H
