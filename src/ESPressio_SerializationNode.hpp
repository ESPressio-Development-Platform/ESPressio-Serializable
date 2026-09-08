#pragma once

#include <algorithm>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "ESPressio_Allocator.hpp"

namespace ESPressio::Serializable {

/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
class SerializationNodeType : uint8_t {
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

/**
 * ESPressio Memory Audit
 * Members:
 * - _type (SerializationNodeType): 1 bytes [0 bytes dynamic allocation]
 * - _booleanValue (bool): 1 bytes [0 bytes dynamic allocation]
 * - _signedIntegerValue (int64_t): 8 bytes [0 bytes dynamic allocation]
 * - _unsignedIntegerValue (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - _float32Value (float): 4 bytes [0 bytes dynamic allocation]
 * - _float64Value (double): 8 bytes [0 bytes dynamic allocation]
 * - _stringValue (SerializationString): sizeof(SerializationString) [0 bytes dynamic allocation]
 * - _objectChildren (std::vector<NamedChild, SerializationAllocator<NamedChild>>): 12 bytes [Capacity * sizeof(NamedChild)]
 * - _arrayChildren (std::vector<SerializationNode, SerializationAllocator<SerializationNode>>): 12 bytes [Capacity * 54 bytes known members + sizeof(SerializationString); N elements each may add: _objectChildren: Capacity * sizeof(NamedChild) + _arrayChildren: Capacity * sizeof(SerializationNode)]
 * Total Memory: 54 bytes known members + sizeof(SerializationString) [_objectChildren: Capacity * sizeof(NamedChild); _arrayChildren: Capacity * 54 bytes known members + sizeof(SerializationString); _arrayChildren: N elements each may add: _objectChildren: Capacity * sizeof(NamedChild) + _arrayChildren: Capacity * sizeof(SerializationNode)]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class SerializationNode {
        public:
            using NamedChild = std::pair<SerializationString, SerializationNode>;

        private:
            SerializationNodeType _type = SerializationNodeType::Null;
            bool _booleanValue = false;
            int64_t _signedIntegerValue = 0;
            uint64_t _unsignedIntegerValue = 0;
            float _float32Value = 0.0f;
            double _float64Value = 0.0;
            SerializationString _stringValue;
            std::vector<NamedChild, SerializationAllocator<NamedChild>> _objectChildren;
            std::vector<SerializationNode, SerializationAllocator<SerializationNode>> _arrayChildren;

            static bool NameEquals(
                const SerializationString& stored,
                std::string_view candidate
            ) noexcept {
                return std::string_view(stored.data(), stored.size()) == candidate;
            }

        public:
            SerializationNode() = default;

            explicit SerializationNode(SerializationNodeType type)
                : _type(type) {
            }

            SerializationNodeType GetType() const {
                return _type;
            }

            void SetType(SerializationNodeType type) {
                _type = type;
                if (type != SerializationNodeType::Object) {
                    _objectChildren.clear();
                }
                if (type != SerializationNodeType::Array) {
                    _arrayChildren.clear();
                }
            }

            bool& BooleanValue() { return _booleanValue; }
            bool BooleanValue() const { return _booleanValue; }
            int64_t& SignedIntegerValue() { return _signedIntegerValue; }
            int64_t SignedIntegerValue() const { return _signedIntegerValue; }
            uint64_t& UnsignedIntegerValue() { return _unsignedIntegerValue; }
            uint64_t UnsignedIntegerValue() const { return _unsignedIntegerValue; }
            float& Float32Value() { return _float32Value; }
            float Float32Value() const { return _float32Value; }
            double& Float64Value() { return _float64Value; }
            double Float64Value() const { return _float64Value; }
            SerializationString& StringValue() { return _stringValue; }
            const SerializationString& StringValue() const { return _stringValue; }

            std::vector<NamedChild, SerializationAllocator<NamedChild>>& ObjectChildren() {
                return _objectChildren;
            }

            const std::vector<NamedChild, SerializationAllocator<NamedChild>>& ObjectChildren() const {
                return _objectChildren;
            }

            std::vector<SerializationNode, SerializationAllocator<SerializationNode>>& ArrayChildren() {
                return _arrayChildren;
            }

            const std::vector<SerializationNode, SerializationAllocator<SerializationNode>>& ArrayChildren() const {
                return _arrayChildren;
            }

            SerializationNode* Find(std::string_view name) {
                if (_type != SerializationNodeType::Object) return nullptr;
                for (auto& child : _objectChildren) {
                    if (NameEquals(child.first, name)) return &child.second;
                }
                return nullptr;
            }

            const SerializationNode* Find(std::string_view name) const {
                if (_type != SerializationNodeType::Object) return nullptr;
                for (const auto& child : _objectChildren) {
                    if (NameEquals(child.first, name)) return &child.second;
                }
                return nullptr;
            }

            SerializationNode& Set(
                std::string_view name,
                SerializationNode node
            ) {
                SetType(SerializationNodeType::Object);
                for (auto& child : _objectChildren) {
                    if (NameEquals(child.first, name)) {
                        child.second = std::move(node);
                        return child.second;
                    }
                }

                SerializationString retainedName;
                retainedName.assign(name.data(), name.size());
                _objectChildren.emplace_back(
                    std::move(retainedName),
                    std::move(node)
                );
                return _objectChildren.back().second;
            }

            SerializationNode& SetOwned(
                SerializationString&& name,
                SerializationNode node
            ) {
                SetType(SerializationNodeType::Object);
                const std::string_view nameView(name.data(), name.size());
                for (auto& child : _objectChildren) {
                    if (NameEquals(child.first, nameView)) {
                        child.second = std::move(node);
                        return child.second;
                    }
                }

                _objectChildren.emplace_back(
                    std::move(name),
                    std::move(node)
                );
                return _objectChildren.back().second;
            }

            SerializationNode& Append(SerializationNode node) {
                SetType(SerializationNodeType::Array);
                _arrayChildren.emplace_back(std::move(node));
                return _arrayChildren.back();
            }

            // A/B regression control: retain the decoder API/type transition while
            // deliberately restoring pre-eager-reserve incremental vector growth.
            void ReserveObject(size_t) {
                SetType(SerializationNodeType::Object);
            }

            void ReserveArray(size_t) {
                SetType(SerializationNodeType::Array);
            }

            bool Remove(std::string_view name) {
                if (_type != SerializationNodeType::Object) return false;
                const auto iterator = std::find_if(
                    _objectChildren.begin(),
                    _objectChildren.end(),
                    [&](const NamedChild& child) {
                        return NameEquals(child.first, name);
                    }
                );
                if (iterator == _objectChildren.end()) return false;
                _objectChildren.erase(iterator);
                return true;
            }

            bool Take(std::string_view name, SerializationNode& output) {
                if (_type != SerializationNodeType::Object) return false;
                const auto iterator = std::find_if(
                    _objectChildren.begin(),
                    _objectChildren.end(),
                    [&](const NamedChild& child) {
                        return NameEquals(child.first, name);
                    }
                );
                if (iterator == _objectChildren.end()) return false;
                output = std::move(iterator->second);
                _objectChildren.erase(iterator);
                return true;
            }
    };

}
