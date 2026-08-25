#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#if defined(ESP32) && defined(ESPRESSIO_SERIALIZATION_PREFER_PSRAM)
    #include "ESPressio_Esp32Allocator.hpp"
#endif

#ifndef ESPRESSIO_SERIALIZATION_ALLOCATOR
    #if defined(ESP32) && defined(ESPRESSIO_SERIALIZATION_PREFER_PSRAM)
        #define ESPRESSIO_SERIALIZATION_ALLOCATOR ESPressio::Serializable::Esp32PreferPsramAllocator
    #else
        #define ESPRESSIO_SERIALIZATION_ALLOCATOR std::allocator
    #endif
#endif

namespace ESPressio::Serializable {

template<typename T>
using SerializationAllocator = ESPRESSIO_SERIALIZATION_ALLOCATOR<T>;

template<typename T>
using SerializationBuffer = std::vector<T, SerializationAllocator<T>>;

}
