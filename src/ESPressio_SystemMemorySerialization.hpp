#pragma once

#include <cstddef>
#include <string>

#include <ESPressio_Memory.hpp>

#include "ESPressio_SerializationAdapter.hpp"
#include "ESPressio_SerializationNode.hpp"

namespace ESPressio::Serializable {

/// <summary>Serializes ESPressio System allocator-aware strings without changing their memory policy.</summary>
/// <remarks>
/// The adapter preserves the caller's allocator on deserialization. This allows persistent configuration,
/// protocol metadata, and other non-DMA text to remain in externally preferred storage while retaining the
/// same serialized string representation used by <c>std::string</c>.
/// </remarks>
template<System::Memory::MemoryPolicy TPolicy>
struct SerializationAdapter<System::Memory::String<TPolicy>> {
    /// <summary>Indicates that System allocator-aware strings are supported by generic serialization traversal.</summary>
    static constexpr bool Supported = true;

    /// <summary>Converts a System-backed string to a serialization string node.</summary>
    static SerializationNode ToNode(const System::Memory::String<TPolicy>& value) {
        SerializationNode node(SerializationNodeType::String);
        node.StringValue().assign(value.data(), value.size());
        return node;
    }

    /// <summary>Copies a serialization string node into the destination while preserving its System allocator policy.</summary>
    static bool FromNode(
        const SerializationNode& node,
        System::Memory::String<TPolicy>& value
    ) {
        if (node.GetType() != SerializationNodeType::String) {
            return false;
        }
        const auto& source = node.StringValue();
        value.assign(source.data(), source.size());
        return true;
    }
};

} // namespace ESPressio::Serializable
