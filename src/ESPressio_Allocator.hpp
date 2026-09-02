#pragma once

#include <cstddef>
#include <vector>

#include <ESPressio_Memory.hpp>

namespace ESPressio::Serializable {

/// <summary>Memory policy used for transient serialization storage that can reside outside internal DRAM.</summary>
inline constexpr auto SerializationMemoryPolicy =
    System::Memory::MemoryPolicy::ExternalPreferred;

/// <summary>Allocator used for transient serialization storage.</summary>
/// <remarks>Serialization storage is routed through ESPressio-System so platform memory policy, PSRAM preference, fallback behavior, and allocation accounting have one authority.</remarks>
template<typename T>
using SerializationAllocator = System::Memory::Allocator<
    T,
    SerializationMemoryPolicy
>;

/// <summary>Vector-like transient serialization buffer using ESPressio-System external-preferred storage.</summary>
template<typename T>
using SerializationBuffer = std::vector<T, SerializationAllocator<T>>;

/// <summary>Transient serialization string whose dynamic storage prefers external memory.</summary>
using SerializationString = System::Memory::String<SerializationMemoryPolicy>;

} // namespace ESPressio::Serializable
