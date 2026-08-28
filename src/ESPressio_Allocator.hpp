#pragma once

#include <cstddef>
#include <vector>

#include <ESPressio_Memory.hpp>

namespace ESPressio::Serializable {

/// <summary>Allocator used for transient serialization storage.</summary>
/// <remarks>Serialization storage is routed through ESPressio-System so platform memory policy, PSRAM preference, fallback behavior, and allocation accounting have one authority.</remarks>
template<typename T>
using SerializationAllocator = System::Memory::Allocator<
    T,
    System::Memory::MemoryPolicy::ExternalPreferred
>;

/// <summary>Vector-like transient serialization buffer using ESPressio-System external-preferred storage.</summary>
template<typename T>
using SerializationBuffer = std::vector<T, SerializationAllocator<T>>;

}
