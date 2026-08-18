#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ESPressio::Serializable {

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

    class SerializationNode {
        public:
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

            std::vector<NamedChild> _objectChildren;
            std::vector<SerializationNode> _arrayChildren;

        public:
            SerializationNode() = default;

            explicit SerializationNode(
                SerializationNodeType type
            )
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

            bool& BooleanValue() {
                return _booleanValue;
            }

            bool BooleanValue() const {
                return _booleanValue;
            }

            int64_t& SignedIntegerValue() {
                return _signedIntegerValue;
            }

            int64_t SignedIntegerValue() const {
                return _signedIntegerValue;
            }

            uint64_t& UnsignedIntegerValue() {
                return _unsignedIntegerValue;
            }

            uint64_t UnsignedIntegerValue() const {
                return _unsignedIntegerValue;
            }

            float& Float32Value() {
                return _float32Value;
            }

            float Float32Value() const {
                return _float32Value;
            }

            double& Float64Value() {
                return _float64Value;
            }

            double Float64Value() const {
                return _float64Value;
            }

            std::string& StringValue() {
                return _stringValue;
            }

            const std::string& StringValue() const {
                return _stringValue;
            }

            std::vector<NamedChild>& ObjectChildren() {
                return _objectChildren;
            }

            const std::vector<NamedChild>& ObjectChildren() const {
                return _objectChildren;
            }

            std::vector<SerializationNode>& ArrayChildren() {
                return _arrayChildren;
            }

            const std::vector<SerializationNode>& ArrayChildren() const {
                return _arrayChildren;
            }

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

            SerializationNode& Append(
                SerializationNode node
            ) {
                SetType(SerializationNodeType::Array);

                _arrayChildren.emplace_back(
                    std::move(node)
                );

                return _arrayChildren.back();
            }
    };

}
