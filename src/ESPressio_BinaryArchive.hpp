#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <utility>

#include "ESPressio_Allocator.hpp"
#include "ESPressio_TreeArchive.hpp"

namespace ESPressio::Serializable {

struct BinaryArchiveDecodeLimits {
    std::size_t MaximumDepth = 32;
    std::size_t MaximumTotalNodes = 4096;
    uint32_t MaximumObjectMembers = 1024;
    uint32_t MaximumArrayElements = 4096;
    std::size_t MaximumNameLength = 1024;
    std::size_t MaximumStringLength = 64u * 1024u;
};

class BinaryArchive : public TreeArchive {
private:
    struct DecodeState {
        const BinaryArchiveDecodeLimits& Limits;
        std::size_t TotalNodes = 0;
    };

    bool _valid = true;

    static std::size_t Remaining(const uint8_t* cursor, const uint8_t* end) noexcept {
        if (cursor == nullptr || end == nullptr || cursor > end) return 0;
        return static_cast<std::size_t>(end - cursor);
    }

    static void AppendU16(SerializationBuffer<uint8_t>& output, uint16_t value) {
        output.push_back(static_cast<uint8_t>(value & 0xFFu));
        output.push_back(static_cast<uint8_t>((value >> 8u) & 0xFFu));
    }

    static void AppendU32(SerializationBuffer<uint8_t>& output, uint32_t value) {
        for (unsigned shift = 0; shift < 32; shift += 8) {
            output.push_back(static_cast<uint8_t>((value >> shift) & 0xFFu));
        }
    }

    static void AppendU64(SerializationBuffer<uint8_t>& output, uint64_t value) {
        for (unsigned shift = 0; shift < 64; shift += 8) {
            output.push_back(static_cast<uint8_t>((value >> shift) & 0xFFu));
        }
    }

    static bool ReadU16(const uint8_t*& cursor, const uint8_t* end, uint16_t& value) {
        if (Remaining(cursor, end) < 2) return false;
        value = static_cast<uint16_t>(cursor[0]) |
            (static_cast<uint16_t>(cursor[1]) << 8u);
        cursor += 2;
        return true;
    }

    static bool ReadU32(const uint8_t*& cursor, const uint8_t* end, uint32_t& value) {
        if (Remaining(cursor, end) < 4) return false;
        value = 0;
        for (int shift = 0; shift < 32; shift += 8) {
            value |= static_cast<uint32_t>(*cursor++) << shift;
        }
        return true;
    }

    static bool ReadU64(const uint8_t*& cursor, const uint8_t* end, uint64_t& value) {
        if (Remaining(cursor, end) < 8) return false;
        value = 0;
        for (int shift = 0; shift < 64; shift += 8) {
            value |= static_cast<uint64_t>(*cursor++) << shift;
        }
        return true;
    }

    static void EncodeNode(
        SerializationBuffer<uint8_t>& output,
        const SerializationNode& node
    ) {
        output.push_back(static_cast<uint8_t>(node.GetType()));

        switch (node.GetType()) {
            case SerializationNodeType::Null:
                break;

            case SerializationNodeType::Object:
                AppendU16(output, static_cast<uint16_t>(node.ObjectChildren().size()));
                for (const auto& child : node.ObjectChildren()) {
                    AppendU16(output, static_cast<uint16_t>(child.first.size()));
                    output.insert(output.end(), child.first.begin(), child.first.end());
                    EncodeNode(output, child.second);
                }
                break;

            case SerializationNodeType::Array:
                AppendU32(output, static_cast<uint32_t>(node.ArrayChildren().size()));
                for (const auto& child : node.ArrayChildren()) EncodeNode(output, child);
                break;

            case SerializationNodeType::Boolean:
                output.push_back(node.BooleanValue() ? 1u : 0u);
                break;

            case SerializationNodeType::SignedInteger: {
                int64_t value = node.SignedIntegerValue();
                uint64_t raw = 0;
                std::memcpy(&raw, &value, sizeof(raw));
                AppendU64(output, raw);
                break;
            }

            case SerializationNodeType::UnsignedInteger:
                AppendU64(output, node.UnsignedIntegerValue());
                break;

            case SerializationNodeType::Float32: {
                uint32_t raw = 0;
                const float value = node.Float32Value();
                std::memcpy(&raw, &value, sizeof(raw));
                AppendU32(output, raw);
                break;
            }

            case SerializationNodeType::Float64: {
                uint64_t raw = 0;
                const double value = node.Float64Value();
                std::memcpy(&raw, &value, sizeof(raw));
                AppendU64(output, raw);
                break;
            }

            case SerializationNodeType::String:
                AppendU32(output, static_cast<uint32_t>(node.StringValue().size()));
                output.insert(output.end(), node.StringValue().begin(), node.StringValue().end());
                break;
        }
    }

    static bool DecodeNode(
        const uint8_t*& cursor,
        const uint8_t* end,
        SerializationNode& node,
        DecodeState& state,
        std::size_t depth
    ) {
        if (cursor == nullptr || end == nullptr || cursor >= end ||
            depth > state.Limits.MaximumDepth ||
            state.TotalNodes >= state.Limits.MaximumTotalNodes) {
            return false;
        }

        ++state.TotalNodes;
        const auto type = static_cast<SerializationNodeType>(*cursor++);
        node.SetType(type);

        switch (type) {
            case SerializationNodeType::Null:
                return true;

            case SerializationNodeType::Object: {
                uint16_t count = 0;
                if (!ReadU16(cursor, end, count) || count > state.Limits.MaximumObjectMembers) {
                    return false;
                }
                node.ReserveObject(count);
                for (uint16_t index = 0; index < count; ++index) {
                    uint16_t nameLength = 0;
                    if (!ReadU16(cursor, end, nameLength) ||
                        nameLength > state.Limits.MaximumNameLength ||
                        Remaining(cursor, end) < nameLength) {
                        return false;
                    }
                    const std::string_view name(
                        reinterpret_cast<const char*>(cursor),
                        nameLength
                    );
                    cursor += nameLength;
                    SerializationNode child;
                    if (!DecodeNode(cursor, end, child, state, depth + 1)) return false;
                    node.Set(name, std::move(child));
                }
                return true;
            }

            case SerializationNodeType::Array: {
                uint32_t count = 0;
                if (!ReadU32(cursor, end, count) || count > state.Limits.MaximumArrayElements) {
                    return false;
                }
                node.ReserveArray(count);
                for (uint32_t index = 0; index < count; ++index) {
                    SerializationNode child;
                    if (!DecodeNode(cursor, end, child, state, depth + 1)) return false;
                    node.Append(std::move(child));
                }
                return true;
            }

            case SerializationNodeType::Boolean:
                if (Remaining(cursor, end) < 1) return false;
                node.BooleanValue() = *cursor++ != 0;
                return true;

            case SerializationNodeType::SignedInteger: {
                uint64_t raw = 0;
                if (!ReadU64(cursor, end, raw)) return false;
                int64_t value = 0;
                std::memcpy(&value, &raw, sizeof(value));
                node.SignedIntegerValue() = value;
                return true;
            }

            case SerializationNodeType::UnsignedInteger:
                return ReadU64(cursor, end, node.UnsignedIntegerValue());

            case SerializationNodeType::Float32: {
                uint32_t raw = 0;
                if (!ReadU32(cursor, end, raw)) return false;
                float value = 0.0f;
                std::memcpy(&value, &raw, sizeof(value));
                node.Float32Value() = value;
                return true;
            }

            case SerializationNodeType::Float64: {
                uint64_t raw = 0;
                if (!ReadU64(cursor, end, raw)) return false;
                double value = 0.0;
                std::memcpy(&value, &raw, sizeof(value));
                node.Float64Value() = value;
                return true;
            }

            case SerializationNodeType::String: {
                uint32_t size = 0;
                if (!ReadU32(cursor, end, size) ||
                    size > state.Limits.MaximumStringLength ||
                    Remaining(cursor, end) < size) {
                    return false;
                }
                node.StringValue().assign(reinterpret_cast<const char*>(cursor), size);
                cursor += size;
                return true;
            }
        }
        return false;
    }

public:
    BinaryArchive() = default;

    template<typename TAllocator>
    explicit BinaryArchive(const std::vector<uint8_t, TAllocator>& data) {
        Load(data);
    }

    BinaryArchive(const uint8_t* data, size_t size) {
        Load(data, size);
    }

    bool IsValid() const { return _valid; }

    SerializationBuffer<uint8_t> GetData() const {
        SerializationBuffer<uint8_t> output = {'E', 'S', 'P', 'B', 2u};
        EncodeNode(output, _root);
        return output;
    }

    bool Load(
        const uint8_t* data,
        size_t size,
        const BinaryArchiveDecodeLimits& limits
    ) noexcept {
        try {
            Clear();
            if (data == nullptr || size < 6 || limits.MaximumTotalNodes == 0 ||
                data[0] != 'E' || data[1] != 'S' || data[2] != 'P' ||
                data[3] != 'B' || data[4] != 2u) {
                _valid = false;
                return false;
            }

            const uint8_t* cursor = data + 5;
            const uint8_t* end = data + size;
            SerializationNode root;
            DecodeState state{limits};

            _valid = DecodeNode(cursor, end, root, state, 0) &&
                cursor == end &&
                root.GetType() == SerializationNodeType::Object;

            if (_valid) _root = std::move(root);
            else Clear();
            return _valid;
        } catch (...) {
            Clear();
            _valid = false;
            return false;
        }
    }

    bool Load(const uint8_t* data, size_t size) noexcept {
        return Load(data, size, BinaryArchiveDecodeLimits{});
    }

    template<typename TAllocator>
    bool Load(
        const std::vector<uint8_t, TAllocator>& data,
        const BinaryArchiveDecodeLimits& limits
    ) noexcept {
        return Load(data.data(), data.size(), limits);
    }

    template<typename TAllocator>
    bool Load(const std::vector<uint8_t, TAllocator>& data) noexcept {
        return Load(data.data(), data.size(), BinaryArchiveDecodeLimits{});
    }
};

}
