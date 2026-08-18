#pragma once
#include "ESPressio_SerializationNode.hpp"
#include "ESPressio_SerializationTraversal.hpp"

namespace ESPressio::Serializable {

    class TreeArchive {
        protected:
            SerializationNode _root;

        public:
            TreeArchive() {
                _root.SetType(SerializationNodeType::Object);
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

            template<typename TValue>
            bool Read(const char* name, TValue& value) {
                SerializationNode* child = _root.Find(name);
                return child != nullptr && Detail::FromNode(*child, value);
            }
    };

}
