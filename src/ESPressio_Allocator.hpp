#pragma once
#include <cstddef>
#include <memory>
#ifndef ESPRESSIO_SERIALIZATION_ALLOCATOR
#define ESPRESSIO_SERIALIZATION_ALLOCATOR std::allocator
#endif
namespace ESPressio::Serializable { template<typename T> using SerializationAllocator = ESPRESSIO_SERIALIZATION_ALLOCATOR<T>; template<typename T> using SerializationBuffer = std::vector<T,SerializationAllocator<T>>; }
