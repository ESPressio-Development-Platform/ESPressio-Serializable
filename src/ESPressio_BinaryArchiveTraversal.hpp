#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>

#include "ESPressio_BinaryArchive.hpp"

namespace ESPressio::Serializable {

class BinaryArchiveVisitor {
public:
    virtual ~BinaryArchiveVisitor() = default;

    virtual bool OnObjectBegin(
        uint32_t,
        std::size_t
    ) noexcept { return true; }

    virtual bool OnObjectProperty(
        std::string_view,
        uint32_t,
        uint32_t,
        std::size_t
    ) noexcept { return true; }

    virtual bool OnObjectEnd(
        uint32_t,
        std::size_t
    ) noexcept { return true; }

    virtual bool OnArrayBegin(
        uint32_t,
        std::size_t
    ) noexcept { return true; }

    virtual bool OnArrayElement(
        uint32_t,
        uint32_t,
        std::size_t
    ) noexcept { return true; }

    virtual bool OnArrayEnd(
        uint32_t,
        std::size_t
    ) noexcept { return true; }

    virtual bool OnNull(
        std::size_t
    ) noexcept { return true; }

    virtual bool OnBoolean(
        bool,
        std::size_t
    ) noexcept { return true; }

    virtual bool OnSignedInteger(
        int64_t,
        std::size_t
    ) noexcept { return true; }

    virtual bool OnUnsignedInteger(
        uint64_t,
        std::size_t
    ) noexcept { return true; }

    virtual bool OnFloat32(
        float,
        std::size_t
    ) noexcept { return true; }

    virtual bool OnFloat64(
        double,
        std::size_t
    ) noexcept { return true; }

    virtual bool OnString(
        std::string_view,
        std::size_t
    ) noexcept { return true; }
};


namespace BinaryArchiveTraversalDetail {

struct State {
    const BinaryArchiveDecodeLimits& Limits;
    std::size_t TotalNodes = 0;
};

inline std::size_t Remaining(
    const uint8_t* cursor,
    const uint8_t* end
) noexcept {
    if (
        cursor == nullptr ||
        end == nullptr ||
        cursor > end
    ) {
        return 0;
    }

    return static_cast<std::size_t>(end - cursor);
}

inline bool ReadU16(
    const uint8_t*& cursor,
    const uint8_t* end,
    uint16_t& value
) noexcept {
    if (Remaining(cursor, end) < 2) {
        return false;
    }

    value =
        static_cast<uint16_t>(cursor[0]) |
        (static_cast<uint16_t>(cursor[1]) << 8u);

    cursor += 2;
    return true;
}

inline bool ReadU32(
    const uint8_t*& cursor,
    const uint8_t* end,
    uint32_t& value
) noexcept {
    if (Remaining(cursor, end) < 4) {
        return false;
    }

    value = 0;
    for (int shift = 0; shift < 32; shift += 8) {
        value |= static_cast<uint32_t>(*cursor++) << shift;
    }
    return true;
}

inline bool ReadU64(
    const uint8_t*& cursor,
    const uint8_t* end,
    uint64_t& value
) noexcept {
    if (Remaining(cursor, end) < 8) {
        return false;
    }

    value = 0;
    for (int shift = 0; shift < 64; shift += 8) {
        value |= static_cast<uint64_t>(*cursor++) << shift;
    }
    return true;
}

inline bool VisitNode(
    const uint8_t*& cursor,
    const uint8_t* end,
    BinaryArchiveVisitor& visitor,
    State& state,
    std::size_t depth
) noexcept {
    if (
        cursor == nullptr ||
        end == nullptr ||
        cursor >= end ||
        depth > state.Limits.MaximumDepth ||
        state.TotalNodes >= state.Limits.MaximumTotalNodes
    ) {
        return false;
    }

    ++state.TotalNodes;

    const auto type =
        static_cast<SerializationNodeType>(*cursor++);

    switch (type) {
        case SerializationNodeType::Null:
            return visitor.OnNull(depth);

        case SerializationNodeType::Object: {
            uint16_t count = 0;
            if (
                !ReadU16(cursor, end, count) ||
                count > state.Limits.MaximumObjectMembers ||
                !visitor.OnObjectBegin(count, depth)
            ) {
                return false;
            }

            for (uint16_t index = 0; index < count; ++index) {
                uint16_t nameLength = 0;
                if (
                    !ReadU16(cursor, end, nameLength) ||
                    nameLength > state.Limits.MaximumNameLength ||
                    Remaining(cursor, end) < nameLength
                ) {
                    return false;
                }

                const std::string_view name(
                    reinterpret_cast<const char*>(cursor),
                    nameLength
                );
                cursor += nameLength;

                if (
                    !visitor.OnObjectProperty(
                        name,
                        index,
                        count,
                        depth
                    ) ||
                    !VisitNode(
                        cursor,
                        end,
                        visitor,
                        state,
                        depth + 1
                    )
                ) {
                    return false;
                }
            }

            return visitor.OnObjectEnd(count, depth);
        }

        case SerializationNodeType::Array: {
            uint32_t count = 0;
            if (
                !ReadU32(cursor, end, count) ||
                count > state.Limits.MaximumArrayElements ||
                !visitor.OnArrayBegin(count, depth)
            ) {
                return false;
            }

            for (uint32_t index = 0; index < count; ++index) {
                if (
                    !visitor.OnArrayElement(
                        index,
                        count,
                        depth
                    ) ||
                    !VisitNode(
                        cursor,
                        end,
                        visitor,
                        state,
                        depth + 1
                    )
                ) {
                    return false;
                }
            }

            return visitor.OnArrayEnd(count, depth);
        }

        case SerializationNodeType::Boolean:
            if (Remaining(cursor, end) < 1) {
                return false;
            }
            return visitor.OnBoolean(*cursor++ != 0, depth);

        case SerializationNodeType::SignedInteger: {
            uint64_t raw = 0;
            if (!ReadU64(cursor, end, raw)) {
                return false;
            }
            int64_t value = 0;
            std::memcpy(&value, &raw, sizeof(value));
            return visitor.OnSignedInteger(value, depth);
        }

        case SerializationNodeType::UnsignedInteger: {
            uint64_t value = 0;
            return
                ReadU64(cursor, end, value) &&
                visitor.OnUnsignedInteger(value, depth);
        }

        case SerializationNodeType::Float32: {
            uint32_t raw = 0;
            if (!ReadU32(cursor, end, raw)) {
                return false;
            }
            float value = 0.0f;
            std::memcpy(&value, &raw, sizeof(value));
            return visitor.OnFloat32(value, depth);
        }

        case SerializationNodeType::Float64: {
            uint64_t raw = 0;
            if (!ReadU64(cursor, end, raw)) {
                return false;
            }
            double value = 0.0;
            std::memcpy(&value, &raw, sizeof(value));
            return visitor.OnFloat64(value, depth);
        }

        case SerializationNodeType::String: {
            uint32_t length = 0;
            if (
                !ReadU32(cursor, end, length) ||
                length > state.Limits.MaximumStringLength ||
                Remaining(cursor, end) < length
            ) {
                return false;
            }

            const std::string_view value(
                reinterpret_cast<const char*>(cursor),
                length
            );
            cursor += length;
            return visitor.OnString(value, depth);
        }
    }

    return false;
}

} // namespace BinaryArchiveTraversalDetail


inline bool TraverseBinaryArchive(
    const uint8_t* data,
    std::size_t size,
    BinaryArchiveVisitor& visitor,
    const BinaryArchiveDecodeLimits& limits = {}
) noexcept {
    if (
        data == nullptr ||
        size < 6 ||
        limits.MaximumTotalNodes == 0 ||
        data[0] != 'E' ||
        data[1] != 'S' ||
        data[2] != 'P' ||
        data[3] != 'B' ||
        data[4] != 2u
    ) {
        return false;
    }

    const uint8_t* cursor = data + 5;
    const uint8_t* end = data + size;
    BinaryArchiveTraversalDetail::State state{limits};

    if (
        cursor >= end ||
        static_cast<SerializationNodeType>(*cursor) !=
            SerializationNodeType::Object ||
        !BinaryArchiveTraversalDetail::VisitNode(
            cursor,
            end,
            visitor,
            state,
            0
        )
    ) {
        return false;
    }

    return cursor == end;
}


inline bool ValidateBinaryArchive(
    const uint8_t* data,
    std::size_t size,
    const BinaryArchiveDecodeLimits& limits = {}
) noexcept {
    BinaryArchiveVisitor visitor;
    return TraverseBinaryArchive(
        data,
        size,
        visitor,
        limits
    );
}

} // namespace ESPressio::Serializable
