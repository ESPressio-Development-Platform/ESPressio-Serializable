#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ESPressio_Allocator.hpp"
#include "ESPressio_EnumSerialization.hpp"
#include "ESPressio_SerializationAdapter.hpp"
#include "ESPressio_SerializationNode.hpp"
#include "ESPressio_SerializationResult.hpp"
#include "ESPressio_SerializationTraits.hpp"
#include "ESPressio_SerializationTraversal.hpp"

namespace ESPressio::Serializable {

namespace DirectBinaryDetail {

    template<typename TOutput>
    inline void AppendU16(
        TOutput& output,
        uint16_t value
    ) {
        output.push_back(
            static_cast<uint8_t>(
                value & 0xFFu
            )
        );
        output.push_back(
            static_cast<uint8_t>(
                (value >> 8u) & 0xFFu
            )
        );
    }


    template<typename TOutput>
    inline void AppendU32(
        TOutput& output,
        uint32_t value
    ) {
        for (
            unsigned shift = 0;
            shift < 32;
            shift += 8
        ) {
            output.push_back(
                static_cast<uint8_t>(
                    (value >> shift) & 0xFFu
                )
            );
        }
    }


    template<typename TOutput>
    inline void AppendU64(
        TOutput& output,
        uint64_t value
    ) {
        for (
            unsigned shift = 0;
            shift < 64;
            shift += 8
        ) {
            output.push_back(
                static_cast<uint8_t>(
                    (value >> shift) & 0xFFu
                )
            );
        }
    }


    inline bool ReadU16(
        const uint8_t*& cursor,
        const uint8_t* end,
        uint16_t& value
    ) {
        if (
            cursor == nullptr ||
            end == nullptr ||
            end - cursor < 2
        ) {
            return false;
        }

        value =
            static_cast<uint16_t>(cursor[0]) |
            (
                static_cast<uint16_t>(cursor[1])
                << 8u
            );

        cursor += 2;
        return true;
    }


    inline bool ReadU32(
        const uint8_t*& cursor,
        const uint8_t* end,
        uint32_t& value
    ) {
        if (
            cursor == nullptr ||
            end == nullptr ||
            end - cursor < 4
        ) {
            return false;
        }

        value = 0;

        for (
            int shift = 0;
            shift < 32;
            shift += 8
        ) {
            value |=
                static_cast<uint32_t>(
                    *cursor++
                ) << shift;
        }

        return true;
    }


    inline bool ReadU64(
        const uint8_t*& cursor,
        const uint8_t* end,
        uint64_t& value
    ) {
        if (
            cursor == nullptr ||
            end == nullptr ||
            end - cursor < 8
        ) {
            return false;
        }

        value = 0;

        for (
            int shift = 0;
            shift < 64;
            shift += 8
        ) {
            value |=
                static_cast<uint64_t>(
                    *cursor++
                ) << shift;
        }

        return true;
    }


    template<typename TOutput>
    inline void AppendName(
        TOutput& output,
        const char* name
    ) {
        const std::size_t length =
            name == nullptr
                ? 0
                : std::strlen(name);

        AppendU16(
            output,
            static_cast<uint16_t>(
                length
            )
        );

        if (length != 0) {
            output.insert(
                output.end(),
                reinterpret_cast<const uint8_t*>(name),
                reinterpret_cast<const uint8_t*>(name) + length
            );
        }
    }


    template<typename TOutput>
    inline void EncodeNode(
        TOutput& output,
        const SerializationNode& node
    ) {
        output.push_back(
            static_cast<uint8_t>(
                node.GetType()
            )
        );

        switch (node.GetType()) {
            case SerializationNodeType::Null:
                return;

            case SerializationNodeType::Object:
                AppendU16(
                    output,
                    static_cast<uint16_t>(
                        node.ObjectChildren().size()
                    )
                );

                for (
                    const auto& child :
                    node.ObjectChildren()
                ) {
                    AppendName(
                        output,
                        child.first.c_str()
                    );
                    EncodeNode(
                        output,
                        child.second
                    );
                }
                return;

            case SerializationNodeType::Array:
                AppendU32(
                    output,
                    static_cast<uint32_t>(
                        node.ArrayChildren().size()
                    )
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
                return;

            case SerializationNodeType::Boolean:
                output.push_back(
                    node.BooleanValue()
                        ? 1u
                        : 0u
                );
                return;

            case SerializationNodeType::SignedInteger: {
                const int64_t value =
                    node.SignedIntegerValue();
                uint64_t raw = 0;
                std::memcpy(
                    &raw,
                    &value,
                    sizeof(raw)
                );
                AppendU64(
                    output,
                    raw
                );
                return;
            }

            case SerializationNodeType::UnsignedInteger:
                AppendU64(
                    output,
                    node.UnsignedIntegerValue()
                );
                return;

            case SerializationNodeType::Float32: {
                const float value =
                    node.Float32Value();
                uint32_t raw = 0;
                std::memcpy(
                    &raw,
                    &value,
                    sizeof(raw)
                );
                AppendU32(
                    output,
                    raw
                );
                return;
            }

            case SerializationNodeType::Float64: {
                const double value =
                    node.Float64Value();
                uint64_t raw = 0;
                std::memcpy(
                    &raw,
                    &value,
                    sizeof(raw)
                );
                AppendU64(
                    output,
                    raw
                );
                return;
            }

            case SerializationNodeType::String:
                AppendU32(
                    output,
                    static_cast<uint32_t>(
                        node.StringValue().size()
                    )
                );
                output.insert(
                    output.end(),
                    node.StringValue().begin(),
                    node.StringValue().end()
                );
                return;
        }
    }


    template<typename TOutput, typename TValue>
    void EncodeValue(
        TOutput& output,
        const TValue& value
    );


    template<typename TOutput, typename TObject>
    void EncodeObject(
        TOutput& output,
        const TObject& object
    ) {
        output.push_back(
            static_cast<uint8_t>(
                SerializationNodeType::Object
            )
        );

        constexpr std::size_t propertyCount =
            std::tuple_size<
                decltype(
                    TObject::GetSerializableProperties()
                )
            >::value;

        static_assert(
            propertyCount <
                std::numeric_limits<uint16_t>::max(),
            "ESPressio Serializable direct binary format supports at most 65534 properties per object."
        );

        AppendU16(
            output,
            static_cast<uint16_t>(
                propertyCount + 1
            )
        );

        AppendName(
            output,
            "__schemaVersion"
        );
        EncodeValue(
            output,
            TObject::GetSchemaVersion()
        );

        std::apply(
            [&](const auto&... property) {
                (
                    (
                        AppendName(
                            output,
                            property.GetName()
                        ),
                        EncodeValue(
                            output,
                            property.GetValue(object)
                        )
                    ),
                    ...
                );
            },
            TObject::GetSerializableProperties()
        );
    }


    template<typename TOutput, typename TValue>
    void EncodeValue(
        TOutput& output,
        const TValue& value
    ) {
        using T =
            std::remove_cv_t<
                std::remove_reference_t<TValue>
            >;

        if constexpr (HasSerializationAdapter<T>) {
            EncodeNode(
                output,
                SerializationAdapter<T>::ToNode(
                    value
                )
            );
        } else if constexpr (IsSerializable<T>) {
            EncodeObject(
                output,
                value
            );
        } else if constexpr (Detail::IsStdOptional<T>::value) {
            if (!value.has_value()) {
                output.push_back(
                    static_cast<uint8_t>(
                        SerializationNodeType::Null
                    )
                );
            } else {
                EncodeValue(
                    output,
                    *value
                );
            }
        } else if constexpr (Detail::IsSequence<T>) {
            output.push_back(
                static_cast<uint8_t>(
                    SerializationNodeType::Array
                )
            );
            AppendU32(
                output,
                static_cast<uint32_t>(
                    value.size()
                )
            );
            for (const auto& item : value) {
                EncodeValue(
                    output,
                    item
                );
            }
        } else if constexpr (Detail::IsMapLike<T>) {
            output.push_back(
                static_cast<uint8_t>(
                    SerializationNodeType::Array
                )
            );
            AppendU32(
                output,
                static_cast<uint32_t>(
                    value.size()
                )
            );

            for (const auto& item : value) {
                output.push_back(
                    static_cast<uint8_t>(
                        SerializationNodeType::Object
                    )
                );
                AppendU16(
                    output,
                    2
                );

                AppendName(
                    output,
                    "key"
                );
                EncodeValue(
                    output,
                    item.first
                );

                AppendName(
                    output,
                    "value"
                );
                EncodeValue(
                    output,
                    item.second
                );
            }
        } else if constexpr (std::is_enum_v<T>) {
            if constexpr (HasEnumSerializationMapping<T>) {
                const char* name =
                    EnumToString(value);
                if (name != nullptr) {
                    output.push_back(
                        static_cast<uint8_t>(
                            SerializationNodeType::String
                        )
                    );
                    const std::size_t length =
                        std::strlen(name);
                    AppendU32(
                        output,
                        static_cast<uint32_t>(
                            length
                        )
                    );
                    output.insert(
                        output.end(),
                        reinterpret_cast<const uint8_t*>(name),
                        reinterpret_cast<const uint8_t*>(name) + length
                    );
                    return;
                }
            }

            EncodeValue(
                output,
                static_cast<
                    std::underlying_type_t<T>
                >(value)
            );
        } else if constexpr (std::is_same_v<T, bool>) {
            output.push_back(
                static_cast<uint8_t>(
                    SerializationNodeType::Boolean
                )
            );
            output.push_back(
                value
                    ? 1u
                    : 0u
            );
        } else if constexpr (
            std::is_integral_v<T> &&
            std::is_signed_v<T>
        ) {
            output.push_back(
                static_cast<uint8_t>(
                    SerializationNodeType::SignedInteger
                )
            );
            const int64_t signedValue =
                static_cast<int64_t>(
                    value
                );
            uint64_t raw = 0;
            std::memcpy(
                &raw,
                &signedValue,
                sizeof(raw)
            );
            AppendU64(
                output,
                raw
            );
        } else if constexpr (std::is_integral_v<T>) {
            output.push_back(
                static_cast<uint8_t>(
                    SerializationNodeType::UnsignedInteger
                )
            );
            AppendU64(
                output,
                static_cast<uint64_t>(
                    value
                )
            );
        } else if constexpr (std::is_same_v<T, float>) {
            output.push_back(
                static_cast<uint8_t>(
                    SerializationNodeType::Float32
                )
            );
            uint32_t raw = 0;
            std::memcpy(
                &raw,
                &value,
                sizeof(raw)
            );
            AppendU32(
                output,
                raw
            );
        } else if constexpr (std::is_floating_point_v<T>) {
            output.push_back(
                static_cast<uint8_t>(
                    SerializationNodeType::Float64
                )
            );
            const double doubleValue =
                static_cast<double>(
                    value
                );
            uint64_t raw = 0;
            std::memcpy(
                &raw,
                &doubleValue,
                sizeof(raw)
            );
            AppendU64(
                output,
                raw
            );
        } else if constexpr (Detail::IsStdString<T>) {
            output.push_back(
                static_cast<uint8_t>(
                    SerializationNodeType::String
                )
            );
            AppendU32(
                output,
                static_cast<uint32_t>(
                    value.size()
                )
            );
            output.insert(
                output.end(),
                value.begin(),
                value.end()
            );
        } else if constexpr (
            std::is_array_v<T> &&
            std::is_same_v<
                std::remove_extent_t<T>,
                char
            >
        ) {
            const std::size_t length =
                std::strlen(value);
            output.push_back(
                static_cast<uint8_t>(
                    SerializationNodeType::String
                )
            );
            AppendU32(
                output,
                static_cast<uint32_t>(
                    length
                )
            );
            output.insert(
                output.end(),
                reinterpret_cast<const uint8_t*>(value),
                reinterpret_cast<const uint8_t*>(value) + length
            );
        } else if constexpr (std::is_pointer_v<T>) {
            static_assert(
                Detail::DependentFalse<T>::value,
                "ESPressio Serializable direct binary: raw pointers are not serializable."
            );
        } else {
            static_assert(
                Detail::DependentFalse<T>::value,
                "ESPressio Serializable direct binary: unsupported property type."
            );
        }
    }


    inline bool SkipNode(
        const uint8_t*& cursor,
        const uint8_t* end
    ) {
        if (
            cursor == nullptr ||
            end == nullptr ||
            cursor >= end
        ) {
            return false;
        }

        const auto type =
            static_cast<SerializationNodeType>(
                *cursor++
            );

        switch (type) {
            case SerializationNodeType::Null:
                return true;

            case SerializationNodeType::Object: {
                uint16_t count = 0;
                if (!ReadU16(cursor, end, count)) {
                    return false;
                }
                for (
                    uint16_t index = 0;
                    index < count;
                    ++index
                ) {
                    uint16_t nameLength = 0;
                    if (
                        !ReadU16(
                            cursor,
                            end,
                            nameLength
                        ) ||
                        end - cursor < nameLength
                    ) {
                        return false;
                    }
                    cursor += nameLength;
                    if (!SkipNode(cursor, end)) {
                        return false;
                    }
                }
                return true;
            }

            case SerializationNodeType::Array: {
                uint32_t count = 0;
                if (!ReadU32(cursor, end, count)) {
                    return false;
                }
                for (
                    uint32_t index = 0;
                    index < count;
                    ++index
                ) {
                    if (!SkipNode(cursor, end)) {
                        return false;
                    }
                }
                return true;
            }

            case SerializationNodeType::Boolean:
                if (cursor >= end) {
                    return false;
                }
                ++cursor;
                return true;

            case SerializationNodeType::SignedInteger:
            case SerializationNodeType::UnsignedInteger:
            case SerializationNodeType::Float64:
                if (end - cursor < 8) {
                    return false;
                }
                cursor += 8;
                return true;

            case SerializationNodeType::Float32:
                if (end - cursor < 4) {
                    return false;
                }
                cursor += 4;
                return true;

            case SerializationNodeType::String: {
                uint32_t length = 0;
                if (
                    !ReadU32(
                        cursor,
                        end,
                        length
                    ) ||
                    length >
                        static_cast<uint32_t>(
                            end - cursor
                        )
                ) {
                    return false;
                }
                cursor += length;
                return true;
            }
        }

        return false;
    }


    inline bool DecodeNode(
        const uint8_t*& cursor,
        const uint8_t* end,
        SerializationNode& node
    ) {
        if (
            cursor == nullptr ||
            end == nullptr ||
            cursor >= end
        ) {
            return false;
        }

        const auto type =
            static_cast<SerializationNodeType>(
                *cursor++
            );

        node.SetType(type);

        switch (type) {
            case SerializationNodeType::Null:
                return true;

            case SerializationNodeType::Object: {
                uint16_t count = 0;
                if (!ReadU16(cursor, end, count)) {
                    return false;
                }
                node.ReserveObject(count);
                for (
                    uint16_t index = 0;
                    index < count;
                    ++index
                ) {
                    uint16_t nameLength = 0;
                    if (
                        !ReadU16(
                            cursor,
                            end,
                            nameLength
                        ) ||
                        end - cursor < nameLength
                    ) {
                        return false;
                    }

                    SerializationString name(
                        reinterpret_cast<const char*>(
                            cursor
                        ),
                        nameLength
                    );
                    cursor += nameLength;

                    SerializationNode child;
                    if (!DecodeNode(cursor, end, child)) {
                        return false;
                    }
                    node.Set(
                        name.c_str(),
                        std::move(child)
                    );
                }
                return true;
            }

            case SerializationNodeType::Array: {
                uint32_t count = 0;
                if (!ReadU32(cursor, end, count)) {
                    return false;
                }
                node.ReserveArray(count);
                for (
                    uint32_t index = 0;
                    index < count;
                    ++index
                ) {
                    SerializationNode child;
                    if (!DecodeNode(cursor, end, child)) {
                        return false;
                    }
                    node.Append(
                        std::move(child)
                    );
                }
                return true;
            }

            case SerializationNodeType::Boolean:
                if (cursor >= end) {
                    return false;
                }
                node.BooleanValue() =
                    *cursor++ != 0;
                return true;

            case SerializationNodeType::SignedInteger: {
                uint64_t raw = 0;
                if (!ReadU64(cursor, end, raw)) {
                    return false;
                }
                int64_t value = 0;
                std::memcpy(
                    &value,
                    &raw,
                    sizeof(value)
                );
                node.SignedIntegerValue() =
                    value;
                return true;
            }

            case SerializationNodeType::UnsignedInteger:
                return ReadU64(
                    cursor,
                    end,
                    node.UnsignedIntegerValue()
                );

            case SerializationNodeType::Float32: {
                uint32_t raw = 0;
                if (!ReadU32(cursor, end, raw)) {
                    return false;
                }
                float value = 0.0f;
                std::memcpy(
                    &value,
                    &raw,
                    sizeof(value)
                );
                node.Float32Value() =
                    value;
                return true;
            }

            case SerializationNodeType::Float64: {
                uint64_t raw = 0;
                if (!ReadU64(cursor, end, raw)) {
                    return false;
                }
                double value = 0.0;
                std::memcpy(
                    &value,
                    &raw,
                    sizeof(value)
                );
                node.Float64Value() =
                    value;
                return true;
            }

            case SerializationNodeType::String: {
                uint32_t length = 0;
                if (
                    !ReadU32(
                        cursor,
                        end,
                        length
                    ) ||
                    length >
                        static_cast<uint32_t>(
                            end - cursor
                        )
                ) {
                    return false;
                }

                node.StringValue().assign(
                    reinterpret_cast<const char*>(
                        cursor
                    ),
                    length
                );
                cursor += length;
                return true;
            }
        }

        return false;
    }


    template<typename TValue>
    bool DecodeValue(
        const uint8_t*& cursor,
        const uint8_t* end,
        TValue& value,
        const DeserializationOptions& options
    );


    class ObjectReaderArchive {
        private:
            struct PropertySlice {
                std::string_view Name;
                const uint8_t* Begin = nullptr;
                const uint8_t* End = nullptr;
            };

            SerializationBuffer<PropertySlice>
                _properties;

            bool _valid = false;

            const PropertySlice*
            FindProperty(
                const char* name
            ) const {
                if (name == nullptr) {
                    return nullptr;
                }

                for (
                    const auto& property :
                    _properties
                ) {
                    if (property.Name == name) {
                        return &property;
                    }
                }

                return nullptr;
            }

        public:
            ObjectReaderArchive(
                const uint8_t* begin,
                const uint8_t* end
            ) {
                if (
                    begin == nullptr ||
                    end == nullptr ||
                    begin >= end
                ) {
                    return;
                }

                const uint8_t* cursor =
                    begin;

                if (
                    static_cast<SerializationNodeType>(
                        *cursor++
                    ) !=
                    SerializationNodeType::Object
                ) {
                    return;
                }

                uint16_t count = 0;
                if (!ReadU16(cursor, end, count)) {
                    return;
                }

                _properties.reserve(count);

                for (
                    uint16_t index = 0;
                    index < count;
                    ++index
                ) {
                    uint16_t nameLength = 0;
                    if (
                        !ReadU16(
                            cursor,
                            end,
                            nameLength
                        ) ||
                        end - cursor < nameLength
                    ) {
                        _properties.clear();
                        return;
                    }

                    const char* nameBegin =
                        reinterpret_cast<const char*>(
                            cursor
                        );
                    cursor += nameLength;

                    const uint8_t* valueBegin =
                        cursor;

                    if (!SkipNode(cursor, end)) {
                        _properties.clear();
                        return;
                    }

                    _properties.push_back(
                        PropertySlice{
                            std::string_view(
                                nameBegin,
                                nameLength
                            ),
                            valueBegin,
                            cursor
                        }
                    );
                }

                _valid =
                    cursor == end;
            }


            bool IsValid() const {
                return _valid;
            }


            bool Contains(
                const char* name
            ) const {
                return
                    _valid &&
                    FindProperty(name) != nullptr;
            }


            template<typename TValue>
            bool Read(
                const char* name,
                TValue& value
            ) {
                const auto* property =
                    FindProperty(name);

                if (property == nullptr) {
                    return false;
                }

                const uint8_t* cursor =
                    property->Begin;

                return
                    DecodeValue(
                        cursor,
                        property->End,
                        value,
                        DeserializationOptions{}
                    ) &&
                    cursor ==
                        property->End;
            }


            template<typename TValue>
            DeserializationResult ReadDetailed(
                const char* name,
                TValue& value,
                const DeserializationOptions& options
            ) {
                DeserializationResult result;

                const auto* property =
                    FindProperty(name);

                if (property == nullptr) {
                    return result;
                }

                const uint8_t* cursor =
                    property->Begin;

                if (
                    !DecodeValue(
                        cursor,
                        property->End,
                        value,
                        options
                    ) ||
                    cursor !=
                        property->End
                ) {
                    result.Add(
                        SerializationErrorCode::TypeMismatch,
                        name == nullptr ? "" : name,
                        "Direct binary value could not be converted to the declared C++ type",
                        options
                    );
                }

                return result;
            }
    };


    template<typename TValue>
    bool DecodeValue(
        const uint8_t*& cursor,
        const uint8_t* end,
        TValue& value,
        const DeserializationOptions& options
    ) {
        using T =
            std::remove_cv_t<
                std::remove_reference_t<TValue>
            >;

        if (
            cursor == nullptr ||
            end == nullptr ||
            cursor >= end
        ) {
            return false;
        }

        if constexpr (HasSerializationAdapter<T>) {
            SerializationNode node;
            if (!DecodeNode(cursor, end, node)) {
                return false;
            }
            return
                SerializationAdapter<T>::FromNode(
                    node,
                    value
                );
        } else if constexpr (IsSerializable<T>) {
            const uint8_t* objectBegin =
                cursor;
            const uint8_t* objectEnd =
                cursor;

            if (!SkipNode(objectEnd, end)) {
                return false;
            }

            ObjectReaderArchive archive(
                objectBegin,
                objectEnd
            );

            if (!archive.IsValid()) {
                return false;
            }

            const DeserializationResult result =
                value.DeserializeDetailed(
                    archive,
                    options
                );

            if (!result.Success()) {
                return false;
            }

            cursor =
                objectEnd;
            return true;
        } else if constexpr (Detail::IsStdOptional<T>::value) {
            if (
                static_cast<SerializationNodeType>(
                    *cursor
                ) ==
                SerializationNodeType::Null
            ) {
                ++cursor;
                value.reset();
                return true;
            }

            typename T::value_type decoded{};
            if (
                !DecodeValue(
                    cursor,
                    end,
                    decoded,
                    options
                )
            ) {
                return false;
            }

            value =
                std::move(decoded);
            return true;
        } else if constexpr (
            Detail::IsStdVector<T>::value ||
            Detail::IsStdDeque<T>::value ||
            Detail::IsStdList<T>::value ||
            Detail::IsStdSet<T>::value ||
            Detail::IsStdUnorderedSet<T>::value
        ) {
            if (
                static_cast<SerializationNodeType>(
                    *cursor++
                ) !=
                SerializationNodeType::Array
            ) {
                return false;
            }

            uint32_t count = 0;
            if (!ReadU32(cursor, end, count)) {
                return false;
            }

            value.clear();

            if constexpr (Detail::IsStdVector<T>::value) {
                value.reserve(count);
            }

            for (
                uint32_t index = 0;
                index < count;
                ++index
            ) {
                typename T::value_type item{};
                if (
                    !DecodeValue(
                        cursor,
                        end,
                        item,
                        options
                    )
                ) {
                    return false;
                }

                if constexpr (
                    Detail::IsStdSet<T>::value ||
                    Detail::IsStdUnorderedSet<T>::value
                ) {
                    value.insert(
                        std::move(item)
                    );
                } else {
                    value.push_back(
                        std::move(item)
                    );
                }
            }

            return true;
        } else if constexpr (Detail::IsStdArray<T>::value) {
            if (
                static_cast<SerializationNodeType>(
                    *cursor++
                ) !=
                SerializationNodeType::Array
            ) {
                return false;
            }

            uint32_t count = 0;
            if (
                !ReadU32(cursor, end, count) ||
                count !=
                    std::tuple_size<T>::value
            ) {
                return false;
            }

            for (
                std::size_t index = 0;
                index < value.size();
                ++index
            ) {
                if (
                    !DecodeValue(
                        cursor,
                        end,
                        value[index],
                        options
                    )
                ) {
                    return false;
                }
            }

            return true;
        } else if constexpr (Detail::IsMapLike<T>) {
            if (
                static_cast<SerializationNodeType>(
                    *cursor++
                ) !=
                SerializationNodeType::Array
            ) {
                return false;
            }

            uint32_t count = 0;
            if (!ReadU32(cursor, end, count)) {
                return false;
            }

            value.clear();

            for (
                uint32_t index = 0;
                index < count;
                ++index
            ) {
                const uint8_t* entryBegin =
                    cursor;
                const uint8_t* entryEnd =
                    cursor;

                if (!SkipNode(entryEnd, end)) {
                    return false;
                }

                ObjectReaderArchive entry(
                    entryBegin,
                    entryEnd
                );

                if (!entry.IsValid()) {
                    return false;
                }

                typename T::key_type key{};
                typename T::mapped_type mapped{};

                if (
                    !entry.Read("key", key) ||
                    !entry.Read("value", mapped)
                ) {
                    return false;
                }

                value.emplace(
                    std::move(key),
                    std::move(mapped)
                );

                cursor =
                    entryEnd;
            }

            return true;
        } else if constexpr (std::is_enum_v<T>) {
            SerializationNode node;
            if (!DecodeNode(cursor, end, node)) {
                return false;
            }
            return
                Detail::FromNode(
                    node,
                    value
                );
        } else if constexpr (std::is_same_v<T, bool>) {
            if (
                static_cast<SerializationNodeType>(
                    *cursor++
                ) !=
                SerializationNodeType::Boolean ||
                cursor >= end
            ) {
                return false;
            }
            value =
                *cursor++ != 0;
            return true;
        } else if constexpr (
            std::is_integral_v<T> &&
            std::is_signed_v<T>
        ) {
            const auto type =
                static_cast<SerializationNodeType>(
                    *cursor++
                );

            if (type == SerializationNodeType::SignedInteger) {
                uint64_t raw = 0;
                if (!ReadU64(cursor, end, raw)) {
                    return false;
                }
                int64_t decoded = 0;
                std::memcpy(
                    &decoded,
                    &raw,
                    sizeof(decoded)
                );
                if (
                    decoded <
                        static_cast<int64_t>(
                            std::numeric_limits<T>::min()
                        ) ||
                    decoded >
                        static_cast<int64_t>(
                            std::numeric_limits<T>::max()
                        )
                ) {
                    return false;
                }
                value =
                    static_cast<T>(
                        decoded
                    );
                return true;
            }

            if (type == SerializationNodeType::UnsignedInteger) {
                uint64_t decoded = 0;
                if (!ReadU64(cursor, end, decoded)) {
                    return false;
                }
                if (
                    decoded >
                        static_cast<uint64_t>(
                            std::numeric_limits<T>::max()
                        )
                ) {
                    return false;
                }
                value =
                    static_cast<T>(
                        decoded
                    );
                return true;
            }

            return false;
        } else if constexpr (
            std::is_integral_v<T> &&
            std::is_unsigned_v<T>
        ) {
            const auto type =
                static_cast<SerializationNodeType>(
                    *cursor++
                );

            if (type == SerializationNodeType::UnsignedInteger) {
                uint64_t decoded = 0;
                if (!ReadU64(cursor, end, decoded)) {
                    return false;
                }
                if (
                    decoded >
                        static_cast<uint64_t>(
                            std::numeric_limits<T>::max()
                        )
                ) {
                    return false;
                }
                value =
                    static_cast<T>(
                        decoded
                    );
                return true;
            }

            if (type == SerializationNodeType::SignedInteger) {
                uint64_t raw = 0;
                if (!ReadU64(cursor, end, raw)) {
                    return false;
                }
                int64_t decoded = 0;
                std::memcpy(
                    &decoded,
                    &raw,
                    sizeof(decoded)
                );
                if (
                    decoded < 0 ||
                    static_cast<uint64_t>(decoded) >
                        static_cast<uint64_t>(
                            std::numeric_limits<T>::max()
                        )
                ) {
                    return false;
                }
                value =
                    static_cast<T>(
                        decoded
                    );
                return true;
            }

            return false;
        } else if constexpr (std::is_same_v<T, float>) {
            const auto type =
                static_cast<SerializationNodeType>(
                    *cursor++
                );

            if (type == SerializationNodeType::Float32) {
                uint32_t raw = 0;
                if (!ReadU32(cursor, end, raw)) {
                    return false;
                }
                std::memcpy(
                    &value,
                    &raw,
                    sizeof(value)
                );
                return true;
            }

            if (type == SerializationNodeType::Float64) {
                uint64_t raw = 0;
                if (!ReadU64(cursor, end, raw)) {
                    return false;
                }
                double decoded = 0.0;
                std::memcpy(
                    &decoded,
                    &raw,
                    sizeof(decoded)
                );
                value =
                    static_cast<float>(
                        decoded
                    );
                return true;
            }

            return false;
        } else if constexpr (std::is_floating_point_v<T>) {
            const auto type =
                static_cast<SerializationNodeType>(
                    *cursor++
                );

            if (type == SerializationNodeType::Float64) {
                uint64_t raw = 0;
                if (!ReadU64(cursor, end, raw)) {
                    return false;
                }
                double decoded = 0.0;
                std::memcpy(
                    &decoded,
                    &raw,
                    sizeof(decoded)
                );
                value =
                    static_cast<T>(
                        decoded
                    );
                return true;
            }

            if (type == SerializationNodeType::Float32) {
                uint32_t raw = 0;
                if (!ReadU32(cursor, end, raw)) {
                    return false;
                }
                float decoded = 0.0f;
                std::memcpy(
                    &decoded,
                    &raw,
                    sizeof(decoded)
                );
                value =
                    static_cast<T>(
                        decoded
                    );
                return true;
            }

            return false;
        } else if constexpr (Detail::IsStdString<T>) {
            if (
                static_cast<SerializationNodeType>(
                    *cursor++
                ) !=
                SerializationNodeType::String
            ) {
                return false;
            }

            uint32_t length = 0;
            if (
                !ReadU32(
                    cursor,
                    end,
                    length
                ) ||
                length >
                    static_cast<uint32_t>(
                        end - cursor
                    )
            ) {
                return false;
            }

            value.assign(
                reinterpret_cast<const char*>(
                    cursor
                ),
                length
            );
            cursor += length;
            return true;
        } else {
            static_assert(
                Detail::DependentFalse<T>::value,
                "ESPressio Serializable direct binary: unsupported property type during deserialization."
            );
        }
    }

} // namespace DirectBinaryDetail


/*
 * Append a Serializable object using the existing BinaryArchive ESPB v2 wire
 * format without constructing an intermediate SerializationNode tree.
 *
 * Existing bytes in output are preserved, allowing callers such as Event
 * transport to reserve/write an envelope and serialize directly after it.
 */
template<typename TObject, typename TAllocator>
bool AppendDirectBinary(
    const TObject& object,
    std::vector<uint8_t, TAllocator>& output
) {
    static_assert(
        IsSerializable<TObject>,
        "AppendDirectBinary requires an ESPressio Serializable type."
    );

    try {
        output.push_back('E');
        output.push_back('S');
        output.push_back('P');
        output.push_back('B');
        output.push_back(2u);

        DirectBinaryDetail::EncodeObject(
            output,
            object
        );
        return true;
    } catch (...) {
        return false;
    }
}


template<typename TObject, typename TAllocator>
bool SerializeDirectBinary(
    const TObject& object,
    std::vector<uint8_t, TAllocator>& output
) {
    output.clear();
    return
        AppendDirectBinary(
            object,
            output
        );
}


template<typename TObject>
DeserializationResult DeserializeDirectBinary(
    const uint8_t* data,
    std::size_t size,
    TObject& object,
    const DeserializationOptions& options = {}
) {
    static_assert(
        IsSerializable<TObject>,
        "DeserializeDirectBinary requires an ESPressio Serializable type."
    );

    DeserializationResult result;

    if (
        data == nullptr ||
        size < 8 ||
        data[0] != 'E' ||
        data[1] != 'S' ||
        data[2] != 'P' ||
        data[3] != 'B' ||
        data[4] != 2u
    ) {
        result.Add(
            SerializationErrorCode::MalformedInput,
            "",
            "Malformed or unsupported direct binary archive",
            options
        );
        return result;
    }

    const uint8_t* begin =
        data + 5;
    const uint8_t* end =
        data + size;

    DirectBinaryDetail::ObjectReaderArchive
        archive(
            begin,
            end
        );

    if (!archive.IsValid()) {
        result.Add(
            SerializationErrorCode::MalformedInput,
            "",
            "Malformed or truncated direct binary object",
            options
        );
        return result;
    }

    return
        object.DeserializeDetailed(
            archive,
            options
        );
}

} // namespace ESPressio::Serializable
