#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>
#include "ESPressio_Allocator.hpp"

namespace ESPressio::Serializable {

    /// <summary>Identifies the value category stored by a <c>SerializationNode</c>.</summary>
    enum class SerializationNodeType : uint8_t {
        Null,
        Object,
        Array,
        Boolean,
        SignedInteger,
        UnsignedInteger,
        Float32,
        Float64,
        String
    };

    /// <summary>Transport-neutral tree node used as the common in-memory representation for serialized values.</summary>
    class SerializationNode {
        public:
            /// <summary>Named child entry stored by object nodes.</summary>
            using NamedChild =
                std::pair<std::string, SerializationNode>;

        private:
            SerializationNodeType _type =
                SerializationNodeType::Null;

            bool _booleanValue = false;
            int64_t _signedIntegerValue = 0;
            uint64_t _unsignedIntegerValue = 0;
            float _float32Value = 0.0f;
            double _float64Value = 0.0;
            std::string _stringValue;

            std::vector<NamedChild, SerializationAllocator<NamedChild>> _objectChildren;
            std::vector<SerializationNode, SerializationAllocator<SerializationNode>> _arrayChildren;

        public:
            /// <summary>Creates a null serialization node.</summary>
            SerializationNode() = default;

            /// <summary>Creates a serialization node with the specified value category.</summary>
            explicit SerializationNode(
                SerializationNodeType type
            )
                : _type(type) {
            }

            /// <summary>Returns the node's current value category.</summary>
            SerializationNodeType GetType() const {
                return _type;
            }

            /// <summary>Changes the node value category and clears incompatible object or array children.</summary>
            void SetType(SerializationNodeType type) {
                _type = type;

                if (type != SerializationNodeType::Object) {
                    _objectChildren.clear();
                }

                if (type != SerializationNodeType::Array) {
                    _arrayChildren.clear();
                }
            }

            /// <summary>Returns mutable access to the node's Boolean storage.</summary>
            bool& BooleanValue() {
                return _booleanValue;
            }

            /// <summary>Returns the node's Boolean storage value.</summary>
            bool BooleanValue() const {
                return _booleanValue;
            }

            /// <summary>Returns mutable access to the node's signed-integer storage.</summary>
            int64_t& SignedIntegerValue() {
                return _signedIntegerValue;
            }

            /// <summary>Returns the node's signed-integer storage value.</summary>
            int64_t SignedIntegerValue() const {
                return _signedIntegerValue;
            }

            /// <summary>Returns mutable access to the node's unsigned-integer storage.</summary>
            uint64_t& UnsignedIntegerValue() {
                return _unsignedIntegerValue;
            }

            /// <summary>Returns the node's unsigned-integer storage value.</summary>
            uint64_t UnsignedIntegerValue() const {
                return _unsignedIntegerValue;
            }

            /// <summary>Returns mutable access to the node's single-precision storage.</summary>
            float& Float32Value() {
                return _float32Value;
            }

            /// <summary>Returns the node's single-precision storage value.</summary>
            float Float32Value() const {
                return _float32Value;
            }

            /// <summary>Returns mutable access to the node's double-precision storage.</summary>
            double& Float64Value() {
                return _float64Value;
            }

            /// <summary>Returns the node's double-precision storage value.</summary>
            double Float64Value() const {
                return _float64Value;
            }

            /// <summary>Returns mutable access to the node's string storage.</summary>
            std::string& StringValue() {
                return _stringValue;
            }

            /// <summary>Returns the node's string storage value.</summary>
            const std::string& StringValue() const {
                return _stringValue;
            }

            /// <summary>Returns mutable access to the named children of an object node.</summary>
            std::vector<NamedChild, SerializationAllocator<NamedChild>>& ObjectChildren() {
                return _objectChildren;
            }

            /// <summary>Returns the named children of an object node.</summary>
            const std::vector<NamedChild, SerializationAllocator<NamedChild>>& ObjectChildren() const {
                return _objectChildren;
            }

            /// <summary>Returns mutable access to the ordered children of an array node.</summary>
            std::vector<SerializationNode, SerializationAllocator<SerializationNode>>& ArrayChildren() {
                return _arrayChildren;
            }

            /// <summary>Returns the ordered children of an array node.</summary>
            const std::vector<SerializationNode, SerializationAllocator<SerializationNode>>& ArrayChildren() const {
                return _arrayChildren;
            }

            /// <summary>Finds a named child of an object node.</summary>
            /// <returns>The child node, or <c>nullptr</c> when not found or when this is not an object node.</returns>
            SerializationNode* Find(const char* name) {
                if (
                    name == nullptr ||
                    _type != SerializationNodeType::Object
                ) {
                    return nullptr;
                }

                for (auto& child : _objectChildren) {
                    if (child.first == name) {
                        return &child.second;
                    }
                }

                return nullptr;
            }

            /// <summary>Finds a named child of an object node.</summary>
            /// <returns>The child node, or <c>nullptr</c> when not found or when this is not an object node.</returns>
            const SerializationNode* Find(
                const char* name
            ) const {
                if (
                    name == nullptr ||
                    _type != SerializationNodeType::Object
                ) {
                    return nullptr;
                }

                for (const auto& child : _objectChildren) {
                    if (child.first == name) {
                        return &child.second;
                    }
                }

                return nullptr;
            }

            /// <summary>Sets or replaces a named child and converts this node to an object node.</summary>
            SerializationNode& Set(
                const char* name,
                SerializationNode node
            ) {
                SetType(SerializationNodeType::Object);

                for (auto& child : _objectChildren) {
                    if (child.first == name) {
                        child.second = std::move(node);
                        return child.second;
                    }
                }

                _objectChildren.emplace_back(
                    name,
                    std::move(node)
                );

                return _objectChildren.back().second;
            }

            /// <summary>Appends a child and converts this node to an array node.</summary>
            SerializationNode& Append(
                SerializationNode node
            ) {
                SetType(SerializationNodeType::Array);

                _arrayChildren.emplace_back(
                    std::move(node)
                );

                return _arrayChildren.back();
            }

            /// <summary>Reserves capacity for named object children and converts this node to an object node.</summary>
            void ReserveObject(size_t count) {
                SetType(SerializationNodeType::Object);
                _objectChildren.reserve(count);
            }

            /// <summary>Reserves capacity for array children and converts this node to an array node.</summary>
            void ReserveArray(size_t count) {
                SetType(SerializationNodeType::Array);
                _arrayChildren.reserve(count);
            }

            /// <summary>Removes a named child from an object node.</summary>
            /// <returns><c>true</c> when a matching child was removed.</returns>
            bool Remove(const char* name) {
                if (name == nullptr || _type != SerializationNodeType::Object) {
                    return false;
                }

                const auto iterator = std::find_if(
                    _objectChildren.begin(),
                    _objectChildren.end(),
                    [&](const NamedChild& child) {
                        return child.first == name;
                    }
                );

                if (iterator == _objectChildren.end()) {
                    return false;
                }

                _objectChildren.erase(iterator);
                return true;
            }

            /// <summary>Moves a named child into an output node and removes it from this object node.</summary>
            /// <returns><c>true</c> when a matching child was found and moved.</returns>
            bool Take(const char* name, SerializationNode& output) {
                if (name == nullptr || _type != SerializationNodeType::Object) {
                    return false;
                }

                const auto iterator = std::find_if(
                    _objectChildren.begin(),
                    _objectChildren.end(),
                    [&](const NamedChild& child) {
                        return child.first == name;
                    }
                );

                if (iterator == _objectChildren.end()) {
                    return false;
                }

                output = std::move(iterator->second);
                _objectChildren.erase(iterator);
                return true;
            }
    };

}
