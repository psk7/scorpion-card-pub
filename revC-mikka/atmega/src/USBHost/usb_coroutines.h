#ifndef SC_MULTICARD_USB_COROUTINES_H
#define SC_MULTICARD_USB_COROUTINES_H

#include "coroutines.h"

namespace USBH {
    struct usb_coro_allocator {
        static void *operator new(std::size_t size) noexcept;

        static void operator delete(void *ptr, std::size_t size);
    };

    template<typename T>
    using task = base_task<T, usb_coro_allocator, std::suspend_never>;

    struct delay_res {};

    std::suspend_always operator co_await(delay_res d) noexcept;

    ////////////////
    template<typename T=uint8_t>
    struct StreamReader {
    public:
        struct promise_type : usb_coro_allocator {
            void *inner;
            promise_type *outer {};
            T m_value;

            StreamReader get_return_object() noexcept {
                return StreamReader(std::coroutine_handle<promise_type>::from_promise(*this));
            }

            std::suspend_always initial_suspend() noexcept { return {}; }

            std::suspend_always final_suspend() noexcept { return {}; }

            std::suspend_always yield_value(T value) noexcept {
                m_value = value;
                outer->inner = nullptr;
                return {};
            }

            void unhandled_exception() {}
        };

        bool await_ready() noexcept { return false; }

        template<typename U>
        void await_suspend(std::coroutine_handle<U> handle) noexcept {
            handle.promise().inner = m_cohandle.address();
            m_cohandle.promise().outer = (promise_type*)&handle.promise();
        }

        uint8_t await_resume() noexcept {
            return m_cohandle.promise().m_value;
        }

        ~StreamReader() {
            if (m_cohandle)
                m_cohandle.destroy();
        }

    private:
        explicit StreamReader(const std::coroutine_handle<promise_type> cohandle)
                : m_cohandle{cohandle} {}

        std::coroutine_handle<promise_type> m_cohandle;
    };
}

#endif //SC_MULTICARD_USB_COROUTINES_H
