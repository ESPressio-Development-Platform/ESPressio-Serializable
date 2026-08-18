#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "ESPressio_TreeArchive.hpp"

namespace ESPressio::Serializable {

    class CborArchive : public TreeArchive {
        private:
            bool _valid = true;

            static void EncodeArgument(
                std::vector<uint8_t>& output,
                uint8_t majorType,
                uint64_t value
            ) {
                const uint8_t prefix =
                    static_cast<uint8_t>(
                        majorType << 5u
                    );

                if (value < 24u) {
                    output.push_back(
                        static_cast<uint8_t>(
                            prefix | value
                        )
                    );
                } else if (value <= 0xFFu) {
                    output.push_back(
                        prefix | 24u
                    );

                    output.push_back(
                        static_cast<uint8_t>(value)
                    );
                } else if (value <= 0xFFFFu) {
                    output.push_back(
                        prefix | 25u
                    );

                    output.push_back(
                        static_cast<uint8_t>(
                            (value >> 8u) & 0xFFu
                        )
                    );

                    output.push_back(
                        static_cast<uint8_t>(
                            value & 0xFFu
                        )
                    );
                } else if (
                    value <= 0xFFFFFFFFull
                ) {
                    output.push_back(
                        prefix | 26u
                    );

                    for (
                        int shift = 24;
                        shift >= 0;
                        shift -= 8
                    ) {
                        output.push_back(
                            static_cast<uint8_t>(
                                (value >> shift) &
                                0xFFu
                            )
                        );
                    }
                } else {
                    output.push_back(
                        prefix | 27u
                    );

                    for (
                        int shift = 56;
                        shift >= 0;
                        shift -= 8
                    ) {
                        output.push_back(
                            static_cast<uint8_t>(
                                (value >> shift) &
                                0xFFu
                            )
                        );
                    }
                }
            }

            static void EncodeText(
                std::vector<uint8_t>& output,
                const std::string& value
            ) {
                EncodeArgument(
                    output,
                    3u,
                    value.size()
                );

                output.insert(
                    output.end(),
                    value.begin(),
                    value.end()
                );
            }

            static void EncodeNode(
                std::vector<uint8_t>& output,
                const SerializationNode& node
            ) {
                switch (node.GetType()) {
                    case SerializationNodeType::Null:
                        output.push_back(0xF6u);
                        break;

                    case SerializationNodeType::Object:
                        EncodeArgument(
                            output,
                            5u,
                            node.ObjectChildren().size()
                        );

                        for (
                            const auto& child :
                            node.ObjectChildren()
                        ) {
                            EncodeText(
                                output,
                                child.first
                            );

                            EncodeNode(
                                output,
                                child.second
                            );
                        }
                        break;

                    case SerializationNodeType::Array:
                        EncodeArgument(
                            output,
                            4u,
                            node.ArrayChildren().size()
                        );

                        for (
                            const auto& child :
                            node.ArrayChildren()
                        ) {
                            EncodeNode(
                                output,
                                child
                            );
                        }
                        break;

                    case SerializationNodeType::Boolean:
                        output.push_back(
                            node.BooleanValue()
                                ? 0xF5u
                                : 0xF4u
                        );
                        break;

                    case SerializationNodeType::SignedInteger:
                        if (
                            node.SignedIntegerValue() >= 0
                        ) {
                            EncodeArgument(
                                output,
                                0u,
                                static_cast<uint64_t>(
                                    node.SignedIntegerValue()
                                )
                            );
                        } else {
                            EncodeArgument(
                                output,
                                1u,
                                static_cast<uint64_t>(
                                    -1 -
                                    node.SignedIntegerValue()
                                )
                            );
                        }
                        break;

                    case SerializationNodeType::UnsignedInteger:
                        EncodeArgument(
                            output,
                            0u,
                            node.UnsignedIntegerValue()
                        );
                        break;

                    case SerializationNodeType::Float32: {
                        output.push_back(0xFAu);

                        uint32_t raw = 0;
                        const float value =
                            node.Float32Value();

                        std::memcpy(
                            &raw,
                            &value,
                            sizeof(raw)
                        );

                        for (
                            int shift = 24;
                            shift >= 0;
                            shift -= 8
                        ) {
                            output.push_back(
                                static_cast<uint8_t>(
                                    (raw >> shift) &
                                    0xFFu
                                )
                            );
                        }
                        break;
                    }

                    case SerializationNodeType::Float64: {
                        output.push_back(0xFBu);

                        uint64_t raw = 0;
                        const double value =
                            node.Float64Value();

                        std::memcpy(
                            &raw,
                            &value,
                            sizeof(raw)
                        );

                        for (
                            int shift = 56;
                            shift >= 0;
                            shift -= 8
                        ) {
                            output.push_back(
                                static_cast<uint8_t>(
                                    (raw >> shift) &
                                    0xFFu
                                )
                            );
                        }
                        break;
                    }

                    case SerializationNodeType::String:
                        EncodeText(
                            output,
                            node.StringValue()
                        );
                        break;
                }
            }

            static bool DecodeArgument(
                const uint8_t*& cursor,
                const uint8_t* end,
                uint8_t additional,
                uint64_t& value
            ) {
                if (additional < 24u) {
                    value = additional;
                    return true;
                }

                size_t byteCount = 0;

                switch (additional) {
                    case 24u: byteCount = 1; break;
                    case 25u: byteCount = 2; break;
                    case 26u: byteCount = 4; break;
                    case 27u: byteCount = 8; break;
                    default: return false;
                }

                if (
                    static_cast<size_t>(
                        end - cursor
                    ) < byteCount
                ) {
                    return false;
                }

                value = 0;

                for (
                    size_t index = 0;
                    index < byteCount;
                    ++index
                ) {
                    value =
                        (value << 8u) |
                        static_cast<uint64_t>(
                            *cursor++
                        );
                }

                return true;
            }

            static bool DecodeNode(
                const uint8_t*& cursor,
                const uint8_t* end,
                SerializationNode& node
            ) {
                if (cursor >= end) {
                    return false;
                }

                const uint8_t initial =
                    *cursor++;

                const uint8_t major =
                    initial >> 5u;

                const uint8_t additional =
                    initial & 0x1Fu;

                if (major == 7u) {
                    if (additional == 20u) {
                        node.SetType(
                            SerializationNodeType::Boolean
                        );

                        node.BooleanValue() = false;
                        return true;
                    }

                    if (additional == 21u) {
                        node.SetType(
                            SerializationNodeType::Boolean
                        );

                        node.BooleanValue() = true;
                        return true;
                    }

                    if (additional == 22u) {
                        node.SetType(
                            SerializationNodeType::Null
                        );

                        return true;
                    }

                    if (additional == 26u) {
                        if (end - cursor < 4) {
                            return false;
                        }

                        uint32_t raw = 0;

                        for (int index = 0; index < 4; ++index) {
                            raw =
                                (raw << 8u) |
                                static_cast<uint32_t>(
                                    *cursor++
                                );
                        }

                        float value = 0.0f;

                        std::memcpy(
                            &value,
                            &raw,
                            sizeof(value)
                        );

                        node.SetType(
                            SerializationNodeType::Float32
                        );

                        node.Float32Value() =
                            value;

                        return true;
                    }

                    if (additional == 27u) {
                        if (end - cursor < 8) {
                            return false;
                        }

                        uint64_t raw = 0;

                        for (int index = 0; index < 8; ++index) {
                            raw =
                                (raw << 8u) |
                                static_cast<uint64_t>(
                                    *cursor++
                                );
                        }

                        double value = 0.0;

                        std::memcpy(
                            &value,
                            &raw,
                            sizeof(value)
                        );

                        node.SetType(
                            SerializationNodeType::Float64
                        );

                        node.Float64Value() =
                            value;

                        return true;
                    }

                    return false;
                }

                uint64_t argument = 0;

                if (
                    !DecodeArgument(
                        cursor,
                        end,
                        additional,
                        argument
                    )
                ) {
                    return false;
                }

                switch (major) {
                    case 0u:
                        node.SetType(
                            SerializationNodeType::UnsignedInteger
                        );

                        node.UnsignedIntegerValue() =
                            argument;

                        return true;

                    case 1u:
                        if (
                            argument >
                            static_cast<uint64_t>(
                                INT64_MAX
                            )
                        ) {
                            return false;
                        }

                        node.SetType(
                            SerializationNodeType::SignedInteger
                        );

                        node.SignedIntegerValue() =
                            -1 -
                            static_cast<int64_t>(
                                argument
                            );

                        return true;

                    case 3u:
                        if (
                            argument >
                            static_cast<uint64_t>(
                                end - cursor
                            )
                        ) {
                            return false;
                        }

                        node.SetType(
                            SerializationNodeType::String
                        );

                        node.StringValue().assign(
                            reinterpret_cast<
                                const char*
                            >(cursor),
                            static_cast<size_t>(
                                argument
                            )
                        );

                        cursor += argument;
                        return true;

                    case 4u:
                        node.SetType(
                            SerializationNodeType::Array
                        );

                        for (
                            uint64_t index = 0;
                            index < argument;
                            ++index
                        ) {
                            SerializationNode child;

                            if (
                                !DecodeNode(
                                    cursor,
                                    end,
                                    child
                                )
                            ) {
                                return false;
                            }

                            node.Append(
                                std::move(child)
                            );
                        }

                        return true;

                    case 5u:
                        node.SetType(
                            SerializationNodeType::Object
                        );

                        for (
                            uint64_t index = 0;
                            index < argument;
                            ++index
                        ) {
                            SerializationNode keyNode;

                            if (
                                !DecodeNode(
                                    cursor,
                                    end,
                                    keyNode
                                ) ||
                                keyNode.GetType() !=
                                    SerializationNodeType::String
                            ) {
                                return false;
                            }

                            SerializationNode valueNode;

                            if (
                                !DecodeNode(
                                    cursor,
                                    end,
                                    valueNode
                                )
                            ) {
                                return false;
                            }

                            node.Set(
                                keyNode.StringValue().c_str(),
                                std::move(valueNode)
                            );
                        }

                        return true;
                }

                return false;
            }

        public:
            CborArchive() = default;

            explicit CborArchive(
                const std::vector<uint8_t>& data
            ) {
                Load(data);
            }

            CborArchive(
                const uint8_t* data,
                size_t size
            ) {
                Load(data, size);
            }

            bool IsValid() const {
                return _valid;
            }

            std::vector<uint8_t> GetData() const {
                std::vector<uint8_t> output;

                EncodeNode(
                    output,
                    _root
                );

                return output;
            }

            bool Load(
                const uint8_t* data,
                size_t size
            ) {
                Clear();

                if (
                    data == nullptr ||
                    size == 0
                ) {
                    _valid = false;
                    return false;
                }

                const uint8_t* cursor = data;
                const uint8_t* end =
                    data + size;

                SerializationNode root;

                _valid =
                    DecodeNode(
                        cursor,
                        end,
                        root
                    ) &&
                    cursor == end &&
                    root.GetType() ==
                        SerializationNodeType::Object;

                if (_valid) {
                    _root = std::move(root);
                }

                return _valid;
            }

            bool Load(
                const std::vector<uint8_t>& data
            ) {
                return Load(
                    data.data(),
                    data.size()
                );
            }
    };

}
