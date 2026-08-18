#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ESPressio_SerializationAdapter.hpp"
#include "ESPressio_SerializationNode.hpp"
#include "ESPressio_SerializationTraits.hpp"

namespace ESPressio::Serializable::Detail {

    template<typename...>
    struct DependentFalse : std::false_type {};

    template<typename T>
    struct IsStdVector : std::false_type {};
    template<typename TValue, typename TAllocator>
    struct IsStdVector<std::vector<TValue, TAllocator>> : std::true_type {};

    template<typename T>
    struct IsStdArray : std::false_type {};
    template<typename TValue, size_t TSize>
    struct IsStdArray<std::array<TValue, TSize>> : std::true_type {};

    template<typename T>
    struct IsStdOptional : std::false_type {};
    template<typename TValue>
    struct IsStdOptional<std::optional<TValue>> : std::true_type {};

    template<typename T>
    struct IsStdMap : std::false_type {};
    template<typename TKey, typename TValue, typename TCompare, typename TAllocator>
    struct IsStdMap<std::map<TKey, TValue, TCompare, TAllocator>>
        : std::true_type {};

    template<typename T>
    struct IsStdUnorderedMap : std::false_type {};
    template<typename TKey, typename TValue, typename THash, typename TEqual, typename TAllocator>
    struct IsStdUnorderedMap<
        std::unordered_map<TKey, TValue, THash, TEqual, TAllocator>
    > : std::true_type {};

    template<typename T>
    inline constexpr bool IsMapLike =
        IsStdMap<std::remove_cv_t<std::remove_reference_t<T>>>::value ||
        IsStdUnorderedMap<std::remove_cv_t<std::remove_reference_t<T>>>::value;

    template<typename T>
    inline constexpr bool IsSequence =
        IsStdVector<std::remove_cv_t<std::remove_reference_t<T>>>::value ||
        IsStdArray<std::remove_cv_t<std::remove_reference_t<T>>>::value;

    template<typename T>
    inline constexpr bool IsStdString =
        std::is_same_v<
            std::remove_cv_t<std::remove_reference_t<T>>,
            std::string
        >;

    class NodeWriterArchive;
    class NodeReaderArchive;

    template<typename TValue>
    SerializationNode ToNode(const TValue& value);

    template<typename TValue>
    bool FromNode(const SerializationNode& node, TValue& value);

    class NodeWriterArchive {
        private:
            SerializationNode& _object;
        public:
            explicit NodeWriterArchive(SerializationNode& object)
                : _object(object) {
                _object.SetType(SerializationNodeType::Object);
            }

            SerializationNode& GetNode() { return _object; }
            const SerializationNode& GetNode() const { return _object; }

            template<typename TValue>
            void Write(const char* name, const TValue& value) {
                _object.Set(name, ToNode(value));
            }
    };

    class NodeReaderArchive {
        private:
            SerializationNode& _object;
        public:
            explicit NodeReaderArchive(SerializationNode& object)
                : _object(object) {}

            SerializationNode& GetNode() { return _object; }
            const SerializationNode& GetNode() const { return _object; }

            template<typename TValue>
            bool Read(const char* name, TValue& value) {
                SerializationNode* child = _object.Find(name);
                return child != nullptr && FromNode(*child, value);
            }
    };

    template<typename TValue>
    SerializationNode ToNode(const TValue& value) {
        using T = std::remove_cv_t<std::remove_reference_t<TValue>>;
        SerializationNode node;

        if constexpr (HasSerializationAdapter<T>) {
            return SerializationAdapter<T>::ToNode(value);
        } else if constexpr (IsSerializable<T>) {
            node.SetType(SerializationNodeType::Object);
            NodeWriterArchive archive(node);
            value.Serialize(archive);
        } else if constexpr (IsStdOptional<T>::value) {
            if (!value.has_value()) {
                node.SetType(SerializationNodeType::Null);
            } else {
                return ToNode(*value);
            }
        } else if constexpr (IsSequence<T>) {
            node.SetType(SerializationNodeType::Array);
            for (const auto& item : value) {
                node.Append(ToNode(item));
            }
        } else if constexpr (IsMapLike<T>) {
            node.SetType(SerializationNodeType::Array);
            for (const auto& item : value) {
                SerializationNode entry(SerializationNodeType::Object);
                entry.Set("key", ToNode(item.first));
                entry.Set("value", ToNode(item.second));
                node.Append(std::move(entry));
            }
        } else if constexpr (std::is_enum_v<T>) {
            using Underlying = std::underlying_type_t<T>;
            return ToNode(static_cast<Underlying>(value));
        } else if constexpr (std::is_same_v<T, bool>) {
            node.SetType(SerializationNodeType::Boolean);
            node.BooleanValue() = value;
        } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
            node.SetType(SerializationNodeType::SignedInteger);
            node.SignedIntegerValue() = static_cast<int64_t>(value);
        } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
            node.SetType(SerializationNodeType::UnsignedInteger);
            node.UnsignedIntegerValue() = static_cast<uint64_t>(value);
        } else if constexpr (std::is_same_v<T, float>) {
            node.SetType(SerializationNodeType::Float32);
            node.Float32Value() = value;
        } else if constexpr (std::is_floating_point_v<T>) {
            node.SetType(SerializationNodeType::Float64);
            node.Float64Value() = static_cast<double>(value);
        } else if constexpr (IsStdString<T>) {
            node.SetType(SerializationNodeType::String);
            node.StringValue() = value;
        } else if constexpr (
            std::is_array_v<T> &&
            std::is_same_v<std::remove_extent_t<T>, char>
        ) {
            node.SetType(SerializationNodeType::String);
            node.StringValue() = value;
        } else if constexpr (std::is_pointer_v<T>) {
            static_assert(
                DependentFalse<T>::value,
                "ESPressio Serializable: raw pointers are not serializable. "
                "Serialize the pointed-to value, use std::optional<T>, or "
                "provide SerializationAdapter<T>."
            );
        } else if constexpr (std::is_class_v<T>) {
            static_assert(
                DependentFalse<T>::value,
                "ESPressio Serializable: unsupported class property type. "
                "Derive the type from Serializable<T>, use a supported STL "
                "container, or specialize SerializationAdapter<T>."
            );
        } else {
            static_assert(
                DependentFalse<T>::value,
                "ESPressio Serializable: unsupported property type. "
                "Provide SerializationAdapter<T> if a custom conversion is required."
            );
        }

        return node;
    }

    template<typename TValue>
    bool FromNode(const SerializationNode& node, TValue& value) {
        using T = std::remove_cv_t<std::remove_reference_t<TValue>>;

        if constexpr (HasSerializationAdapter<T>) {
            return SerializationAdapter<T>::FromNode(node, value);
        } else if constexpr (IsSerializable<T>) {
            if (node.GetType() != SerializationNodeType::Object) {
                return false;
            }
            SerializationNode copy = node;
            NodeReaderArchive archive(copy);
            return value.Deserialize(archive);
        } else if constexpr (IsStdOptional<T>::value) {
            if (node.GetType() == SerializationNodeType::Null) {
                value.reset();
                return true;
            }
            typename T::value_type decoded{};
            if (!FromNode(node, decoded)) {
                return false;
            }
            value = std::move(decoded);
            return true;
        } else if constexpr (IsStdVector<T>::value) {
            if (node.GetType() != SerializationNodeType::Array) {
                return false;
            }
            value.clear();
            value.reserve(node.ArrayChildren().size());
            for (const auto& child : node.ArrayChildren()) {
                typename T::value_type item{};
                if (!FromNode(child, item)) {
                    return false;
                }
                value.push_back(std::move(item));
            }
            return true;
        } else if constexpr (IsStdArray<T>::value) {
            if (
                node.GetType() != SerializationNodeType::Array ||
                node.ArrayChildren().size() != std::tuple_size<T>::value
            ) {
                return false;
            }
            for (size_t i = 0; i < value.size(); ++i) {
                if (!FromNode(node.ArrayChildren()[i], value[i])) {
                    return false;
                }
            }
            return true;
        } else if constexpr (IsMapLike<T>) {
            if (node.GetType() != SerializationNodeType::Array) {
                return false;
            }
            value.clear();
            for (const auto& child : node.ArrayChildren()) {
                if (child.GetType() != SerializationNodeType::Object) {
                    return false;
                }
                const auto* keyNode = child.Find("key");
                const auto* valueNode = child.Find("value");
                if (keyNode == nullptr || valueNode == nullptr) {
                    return false;
                }
                typename T::key_type key{};
                typename T::mapped_type mapped{};
                if (
                    !FromNode(*keyNode, key) ||
                    !FromNode(*valueNode, mapped)
                ) {
                    return false;
                }
                value.emplace(std::move(key), std::move(mapped));
            }
            return true;
        } else if constexpr (std::is_enum_v<T>) {
            using Underlying = std::underlying_type_t<T>;
            Underlying decoded{};
            if (!FromNode(node, decoded)) {
                return false;
            }
            value = static_cast<T>(decoded);
            return true;
        } else if constexpr (std::is_same_v<T, bool>) {
            if (node.GetType() != SerializationNodeType::Boolean) {
                return false;
            }
            value = node.BooleanValue();
            return true;
        } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
            if (node.GetType() == SerializationNodeType::SignedInteger) {
                value = static_cast<T>(node.SignedIntegerValue());
                return true;
            }
            if (node.GetType() == SerializationNodeType::UnsignedInteger) {
                value = static_cast<T>(node.UnsignedIntegerValue());
                return true;
            }
            return false;
        } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
            if (node.GetType() == SerializationNodeType::UnsignedInteger) {
                value = static_cast<T>(node.UnsignedIntegerValue());
                return true;
            }
            if (
                node.GetType() == SerializationNodeType::SignedInteger &&
                node.SignedIntegerValue() >= 0
            ) {
                value = static_cast<T>(node.SignedIntegerValue());
                return true;
            }
            return false;
        } else if constexpr (std::is_same_v<T, float>) {
            if (node.GetType() == SerializationNodeType::Float32) {
                value = node.Float32Value();
                return true;
            }
            if (node.GetType() == SerializationNodeType::Float64) {
                value = static_cast<float>(node.Float64Value());
                return true;
            }
            return false;
        } else if constexpr (std::is_floating_point_v<T>) {
            if (node.GetType() == SerializationNodeType::Float64) {
                value = static_cast<T>(node.Float64Value());
                return true;
            }
            if (node.GetType() == SerializationNodeType::Float32) {
                value = static_cast<T>(node.Float32Value());
                return true;
            }
            return false;
        } else if constexpr (IsStdString<T>) {
            if (node.GetType() != SerializationNodeType::String) {
                return false;
            }
            value = node.StringValue();
            return true;
        } else if constexpr (std::is_pointer_v<T>) {
            static_assert(
                DependentFalse<T>::value,
                "ESPressio Serializable: raw pointers cannot be deserialized. "
                "Use std::optional<T> or provide SerializationAdapter<T>."
            );
        } else if constexpr (std::is_class_v<T>) {
            static_assert(
                DependentFalse<T>::value,
                "ESPressio Serializable: unsupported class property type. "
                "Derive from Serializable<T> or specialize SerializationAdapter<T>."
            );
        } else {
            static_assert(
                DependentFalse<T>::value,
                "ESPressio Serializable: unsupported property type during deserialization."
            );
        }

        return false;
    }

}
