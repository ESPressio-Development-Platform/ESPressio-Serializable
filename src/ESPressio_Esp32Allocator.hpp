#pragma once
#if !defined(ESP32) || !__has_include(<esp_heap_caps.h>)
#error "ESPressio_Esp32Allocator.hpp requires ESP32 heap capabilities."
#endif
#include <cstddef>
#include <new>
#include <esp_heap_caps.h>

namespace ESPressio::Serializable {

template<typename T, uint32_t Caps>
class Esp32HeapCapsAllocator {
public:
    using value_type=T;
    Esp32HeapCapsAllocator() noexcept = default;
    template<typename U> Esp32HeapCapsAllocator(const Esp32HeapCapsAllocator<U,Caps>&) noexcept {}
    T* allocate(std::size_t n){
        void* p=heap_caps_malloc(n*sizeof(T),Caps);
        if(!p) throw std::bad_alloc();
        return static_cast<T*>(p);
    }
    void deallocate(T* p,std::size_t) noexcept { heap_caps_free(p); }
    template<typename U> struct rebind { using other=Esp32HeapCapsAllocator<U,Caps>; };
};

template<typename T> using Esp32InternalAllocator = Esp32HeapCapsAllocator<T, MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT>;
template<typename T> using Esp32PsramAllocator = Esp32HeapCapsAllocator<T, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT>;
template<typename T> using Esp32PreferPsramAllocator = Esp32HeapCapsAllocator<T, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT>;
}
