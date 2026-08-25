#pragma once
#if !defined(ESP32) || !__has_include(<esp_heap_caps.h>)
#error "ESPressio_Esp32Allocator.hpp requires ESP32 heap capabilities."
#endif

#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <esp_heap_caps.h>

namespace ESPressio::Serializable {

template<typename T, uint32_t Caps>
class Esp32HeapCapsAllocator {
public:
    using value_type = T;

    Esp32HeapCapsAllocator() noexcept = default;

    template<typename U>
    Esp32HeapCapsAllocator(const Esp32HeapCapsAllocator<U, Caps>&) noexcept {}

    T* allocate(std::size_t n) {
        if (n > static_cast<std::size_t>(-1) / sizeof(T)) {
            throw std::bad_array_new_length();
        }

        void* p = heap_caps_malloc(n * sizeof(T), Caps);
        if (!p) throw std::bad_alloc();
        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t) noexcept {
        heap_caps_free(p);
    }

    template<typename U>
    struct rebind {
        using other = Esp32HeapCapsAllocator<U, Caps>;
    };
};


template<typename T>
class Esp32PreferPsramAllocator {
public:
    using value_type = T;

    Esp32PreferPsramAllocator() noexcept = default;

    template<typename U>
    Esp32PreferPsramAllocator(const Esp32PreferPsramAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        if (n > static_cast<std::size_t>(-1) / sizeof(T)) {
            throw std::bad_array_new_length();
        }

        const std::size_t bytes = n * sizeof(T);

        void* p = heap_caps_malloc(
            bytes,
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
        );

        if (p == nullptr) {
            p = heap_caps_malloc(
                bytes,
                MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
            );
        }

        if (p == nullptr) {
            throw std::bad_alloc();
        }

        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t) noexcept {
        heap_caps_free(p);
    }

    template<typename U>
    struct rebind {
        using other = Esp32PreferPsramAllocator<U>;
    };
};


template<typename T>
using Esp32InternalAllocator =
    Esp32HeapCapsAllocator<T, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT>;

template<typename T>
using Esp32PsramAllocator =
    Esp32HeapCapsAllocator<T, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT>;


template<typename T, typename U, uint32_t Caps>
constexpr bool operator==(
    const Esp32HeapCapsAllocator<T, Caps>&,
    const Esp32HeapCapsAllocator<U, Caps>&
) noexcept {
    return true;
}

template<typename T, typename U, uint32_t Caps>
constexpr bool operator!=(
    const Esp32HeapCapsAllocator<T, Caps>& a,
    const Esp32HeapCapsAllocator<U, Caps>& b
) noexcept {
    return !(a == b);
}

template<typename T, typename U>
constexpr bool operator==(
    const Esp32PreferPsramAllocator<T>&,
    const Esp32PreferPsramAllocator<U>&
) noexcept {
    return true;
}

template<typename T, typename U>
constexpr bool operator!=(
    const Esp32PreferPsramAllocator<T>& a,
    const Esp32PreferPsramAllocator<U>& b
) noexcept {
    return !(a == b);
}

}
