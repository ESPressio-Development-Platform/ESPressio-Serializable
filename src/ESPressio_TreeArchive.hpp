#pragma once
#include "ESPressio_SerializationNode.hpp"
#include "ESPressio_SerializationTraversal.hpp"
#include "ESPressio_SerializationPolicy.hpp"

namespace ESPressio::Serializable {

    class TreeArchive {
        protected:
            SerializationNode _root;
            SerializationPolicy _policy;

        public:
            TreeArchive() {
                _root.SetType(SerializationNodeType::Object);
            }


            SerializationPolicy& Policy() { return _policy; }
            const SerializationPolicy& Policy() const { return _policy; }

            template<typename TProperty, typename TValue>
            void WriteProperty(const TProperty& property, const TValue& value) {
                if (property.IsSensitive()) {
                    switch (_policy.GetSensitivePolicy()) {
                        case SensitivePropertyPolicy::Omit:
                            return;
                        case SensitivePropertyPolicy::Redact:
                            _root.Set(property.GetName(), Detail::ToNode(std::string(_policy.GetRedactionText())));
                            return;
                        case SensitivePropertyPolicy::Include:
                            break;
                    }
                }
                Write(property.GetName(), value);
            }

            SerializationNode& GetNode() { return _root; }
            const SerializationNode& GetNode() const { return _root; }
            SerializationNode& GetRootNode() { return _root; }
            const SerializationNode& GetRootNode() const { return _root; }

            void Clear() {
                _root = SerializationNode(SerializationNodeType::Object);
            }

            template<typename TValue>
            void Write(const char* name, const TValue& value) {
                _root.Set(name, Detail::ToNode(value));
            }

            bool Contains(const char* name) const {
                return _root.Find(name) != nullptr;
            }

            template<typename TValue>
            bool Read(const char* name, TValue& value) {
                SerializationNode* child = _root.Find(name);
                return child != nullptr && Detail::FromNode(*child, value);
            }
    };

}
