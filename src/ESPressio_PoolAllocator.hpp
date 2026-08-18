#pragma once
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#ifndef ESPRESSIO_SERIALIZATION_STATIC_POOL_BYTES
#define ESPRESSIO_SERIALIZATION_STATIC_POOL_BYTES 16384
#endif
namespace ESPressio::Serializable {
class StaticSerializationPool {
    alignas(std::max_align_t) unsigned char _data[ESPRESSIO_SERIALIZATION_STATIC_POOL_BYTES]{};
    size_t _offset=0;
public:
    void* Allocate(size_t bytes,size_t alignment){
        uintptr_t base=reinterpret_cast<uintptr_t>(_data)+_offset;
        uintptr_t aligned=(base+alignment-1)&~(alignment-1);
        size_t next=static_cast<size_t>(aligned-reinterpret_cast<uintptr_t>(_data))+bytes;
        if(next>sizeof(_data)) throw std::bad_alloc();
        _offset=next; return reinterpret_cast<void*>(aligned);
    }
    void Reset(){_offset=0;}
    size_t Used()const{return _offset;}
    size_t Capacity()const{return sizeof(_data);}
};
inline StaticSerializationPool& SerializationStaticPool(){ static StaticSerializationPool pool; return pool; }
template<typename T> class StaticPoolAllocator {
public:
    using value_type=T;
    StaticPoolAllocator() noexcept=default;
    template<typename U> StaticPoolAllocator(const StaticPoolAllocator<U>&) noexcept{}
    T* allocate(size_t n){return static_cast<T*>(SerializationStaticPool().Allocate(n*sizeof(T),alignof(T)));}
    void deallocate(T*,size_t) noexcept {}
    template<typename U> struct rebind{using other=StaticPoolAllocator<U>;};
};
}
