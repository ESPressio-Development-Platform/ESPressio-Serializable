#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "ESPressio_BoundedDeserializer.hpp"

namespace ESPressio::Serializable {

/**
 * Decodes canonical ESPB v2 directly into caller-owned scratch storage.
 *
 * Unlike DeserializeBoundedDirectBinary(), this API deliberately does not
 * allocate an additional temporary T in order to provide publish-on-success
 * semantics. On failure, `scratch` may be partially modified and MUST NOT be
 * treated as published application state. This makes the storage cost explicit
 * for large bounded objects such as constrained OTA manifests.
 */
template<class T>
BoundedSerializationResult DeserializeBoundedDirectBinaryIntoScratch(
    const std::uint8_t* data,
    std::size_t size,
    T& scratch) noexcept {
    static_assert(IsBoundedSerializable<T>,
                  "Bounded scratch decode requires an explicit schema version and bounded property graph");
    if (size > MaximumSerializedSize<T, DirectBinary>) {
        return {SerializationErrorCode::ResourceLimitExceeded, 0U};
    }
    if (!data || size < 8U || data[0] != 'E' || data[1] != 'S' || data[2] != 'P' ||
        data[3] != 'B' || data[4] != 2U) {
        return {SerializationErrorCode::MalformedInput, 0U};
    }

    DirectBinaryDetail::BoundedReader<> reader{data + 5U, data + size};
    if (!reader.Object(scratch)) return {reader.Error, 0U};
    if (reader.Cursor != reader.End) return {SerializationErrorCode::MalformedInput, 0U};
    return {SerializationErrorCode::None, size};
}

} // namespace ESPressio::Serializable
