#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-dcl58-cpp"
#pragma ide diagnostic ignored "bugprone-dynamic-static-initializers"
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#ifndef SC_MULTICARD_COROUTINES_H
#define SC_MULTICARD_COROUTINES_H

#include <stdint.h>

namespace std {
    typedef __SIZE_TYPE__ size_t;

    template<typename _Result, typename... _ArgTypes>
    struct coroutine_traits;

    template<typename _Result, typename = void>
    struct __coroutine_traits_impl {};

    template<typename _Result>
#if __cpp_concepts
    requires requires { typename _Result::promise_type; }
    struct __coroutine_traits_impl<_Result, void>
#else
        struct __coroutine_traits_impl<_Result,
                   __void_t<typename _Result::promise_type>>
#endif
    {
        using promise_type = typename _Result::promise_type;
    };

    template<typename _Result, typename... _ArgTypes>
    struct coroutine_traits : __coroutine_traits_impl<_Result> {};

    template<class T>
    struct remove_reference;

    template<class T>
    using remove_reference_t = typename remove_reference<T>::type;

    template<class T>
    constexpr std::remove_reference_t<T> &&move(T &&t) noexcept;

    template<typename _Promise = void>
    struct coroutine_handle;

    using nullptr_t = decltype(nullptr);

    template<>
    struct coroutine_handle<void> {
    public:
        constexpr coroutine_handle() noexcept: _M_fr_ptr(nullptr) {}

        constexpr coroutine_handle(std::nullptr_t __h) noexcept: _M_fr_ptr(__h) {}

        coroutine_handle &operator=(std::nullptr_t) noexcept {
            _M_fr_ptr = nullptr;
            return *this;
        }

    public:
        constexpr void *address() const noexcept { return _M_fr_ptr; }

        constexpr static coroutine_handle from_address(void *__a) noexcept {
            coroutine_handle __self;
            __self._M_fr_ptr = __a;
            return __self;
        }

    public:
        constexpr explicit operator bool() const noexcept { return bool(_M_fr_ptr); }

        [[nodiscard]] bool done() const noexcept { return __builtin_coro_done(_M_fr_ptr); }

        void operator()() const { resume(); }

        void resume() const { __builtin_coro_resume(_M_fr_ptr); }

        void destroy() { __builtin_coro_destroy(_M_fr_ptr); }

    protected:
        void *_M_fr_ptr;
    };

    template<typename _Promise>
    struct coroutine_handle {
        constexpr coroutine_handle() noexcept {}

        constexpr coroutine_handle(nullptr_t) noexcept {}

        static coroutine_handle from_promise(_Promise &__p) {
            coroutine_handle __self;
            __self._M_fr_ptr = __builtin_coro_promise((char *) &__p, __alignof(_Promise), true);
            return __self;
        }

        coroutine_handle &operator=(nullptr_t) noexcept {
            _M_fr_ptr = nullptr;
            return *this;
        }

        constexpr void *address() const noexcept { return _M_fr_ptr; }

        constexpr static coroutine_handle from_address(void *__a) noexcept {
            coroutine_handle __self;
            __self._M_fr_ptr = __a;
            return __self;
        }

        constexpr operator coroutine_handle<>() const noexcept { return coroutine_handle<>::from_address(address()); }

        constexpr explicit operator bool() const noexcept { return bool(_M_fr_ptr); }

        [[nodiscard]] bool done() const noexcept { return __builtin_coro_done(_M_fr_ptr); }

        void operator()() const { resume(); }

        void resume() const { __builtin_coro_resume(_M_fr_ptr); }

        void destroy() { __builtin_coro_destroy(_M_fr_ptr); }

        // [coroutine.handle.promise], promise access
        [[nodiscard]] _Promise &promise() const {
            void *__t = __builtin_coro_promise(_M_fr_ptr, __alignof(_Promise), false);
            return *static_cast<_Promise *>(__t);
        }

    private:
        void *_M_fr_ptr = nullptr;
    };


    struct suspend_always {
        constexpr bool await_ready() const noexcept { return false; }

        constexpr void await_suspend(coroutine_handle<>) const noexcept {}

        constexpr void await_resume() const noexcept {}
    };

    struct suspend_never {
        constexpr bool await_ready() const noexcept { return true; }

        constexpr void await_suspend(coroutine_handle<>) const noexcept {}

        constexpr void await_resume() const noexcept {}
    };
}

template<typename T, typename Alloc, typename I>
struct base_task;

template<typename Alloc, typename I>
struct base_task<void, Alloc, I> {
    struct task_promise;

    using promise_type = task_promise;
    using handle_type = std::coroutine_handle<promise_type>;
    using task_type = base_task<void, Alloc, I>;

    mutable handle_type m_handle;

    explicit base_task() : m_handle(nullptr) {}

    base_task(handle_type handle) : m_handle(handle) {}

    base_task(base_task &&other) noexcept: m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }

    task_type &operator=(const task_type &) = default;

    task_type &operator=(const task_type &&src) noexcept {
        m_handle = src.m_handle;
        src.m_handle = {};
        return *this;
    }

    constexpr explicit operator bool() const noexcept { return bool(m_handle); }

    bool done() {
        if (!m_handle)
            return false;
        else
            return m_handle.done();
    }

    void destroy() {
        if (m_handle)
            m_handle.destroy();

        m_handle = {};
    }

    bool await_ready() { return !m_handle || m_handle.done(); }

    template<typename U>
    void await_suspend(std::coroutine_handle<U> handle){
        handle.promise().inner = m_handle.address();
    }

    void await_resume() {}

    ~base_task() { destroy(); }

    struct task_promise : Alloc {
        void *inner {};

        auto initial_suspend() { return I{}; }

        auto final_suspend() noexcept { return std::suspend_always{}; }

        task_type get_return_object() { return {handle_type::from_promise(*this)}; }

        void unhandled_exception() {}

        void rethrow_if_unhandled_exception() {}

        void return_void() {}

        static auto get_return_object_on_allocation_failure() noexcept { return task_type(); }
    };
};

template<typename T, typename Alloc, typename I>
struct base_task {
    struct task_promise;

    using promise_type = task_promise;
    using handle_type = std::coroutine_handle<promise_type>;
    using task_type = base_task<T, Alloc, I>;

    mutable handle_type m_handle;

    base_task(handle_type handle) : m_handle(handle) {}

    explicit base_task() : m_handle(nullptr) {}

    base_task(base_task &&other) noexcept: m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }

    task_type &operator=(const task_type &) = default;

    task_type &operator=(const task_type &&src) {
        m_handle = src.m_handle;
        src.m_handle = {};
        return *this;
    }

    constexpr explicit operator bool() const noexcept { return bool(m_handle); }

    bool done() {
        if (!m_handle)
            return false;
        else
            return m_handle.done();
    }

    void destroy() {
        if (m_handle)
            m_handle.destroy();

        m_handle = {};
    }

    bool await_ready() { return !m_handle || m_handle.done(); }

    template<typename U>
    void await_suspend(std::coroutine_handle<U> handle) { handle.promise().inner = m_handle.address(); }

    auto await_resume() { return m_handle.promise().m_value; }

    void resume() {
        void *p = m_handle.address();
        void *pp = nullptr;
        void *inner;

        while((inner = ((task_promise*)(__builtin_coro_promise(p, __alignof(promise_type), false)))->inner) != nullptr)
        {
            pp = p;
            p = inner;
        }

        auto h = std::coroutine_handle<>::from_address(p);

        h.resume();

        if (!h.done() || pp == nullptr)
            return;

        auto rpromise = ((task_promise*)(__builtin_coro_promise(pp, __alignof(promise_type), false)));
        rpromise->inner = nullptr;
    }

    auto value() { return m_handle.promise().m_value; }

    ~base_task() { destroy(); }

    struct task_promise : Alloc {
        void *inner {};
        T m_value;

        auto value() { return m_value; }

        auto initial_suspend() { return I{}; }

        auto final_suspend() noexcept { return std::suspend_always{}; }

        auto yield_value(T value) noexcept {
            m_value = value;
            return std::suspend_always{};
        }

        auto return_value(T t) {
            m_value = t;
            return std::suspend_always{};
        }

        task_type get_return_object() {
            task_type t = {handle_type::from_promise(*this)};
            return t;
        }

        void unhandled_exception() {}

        void rethrow_if_unhandled_exception() {}

        static auto get_return_object_on_allocation_failure() noexcept { return task_type(); }
    };
};

#endif //SC_MULTICARD_COROUTINES_H

#pragma clang diagnostic pop