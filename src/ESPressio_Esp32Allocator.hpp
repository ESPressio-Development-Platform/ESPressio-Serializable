#pragma once

#include <ESPressio_Memory.hpp>

namespace ESPressio::Serializable {

/// <summary>ESP32-facing allocator name for storage that should prefer external RAM.</summary>
/// <remarks>Allocation policy is implemented exclusively by ESPressio System and the installed ESP32 memory provider; this header no longer calls ESP-IDF heap APIs directly.</remarks>
template<typename T>
using Esp32PreferPsramAllocator = System::Memory::Allocator<
    T,
    System::Memory::MemoryPolicy::ExternalPreferred
>;

/// <summary>ESP32-facing allocator name for storage that must reside in internal RAM.</summary>
template<typename T>
using Esp32InternalAllocator = System::Memory::Allocator<
    T,
    System::Memory::MemoryPolicy::Internal
>;

/// <summary>ESP32-facing allocator name for storage that requires external RAM.</summary>
template<typename T>
using Esp32PsramAllocator = System::Memory::Allocator<
    T,
    System::Memory::MemoryPolicy::ExternalRequired
>;

}