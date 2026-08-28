#pragma once
#include "ESPressio_SerializationNode.hpp"
#include "ESPressio_SerializationTraversal.hpp"
#include "ESPressio_SerializationPolicy.hpp"

namespace ESPressio::Serializable {

    /// <summary>In-memory object archive backed by a <c>SerializationNode</c> tree.</summary>
    /// <remarks>Provides generic typed reads/writes through the shared traversal layer and applies sensitive-property policy during property-aware writes.</remarks>
    class TreeArchive {
        protected:
            SerializationNode _root;
            SerializationPolicy _policy;

        public:
            /// <summary>Creates an empty object-rooted archive.</summary>
            TreeArchive() {
                _root.SetType(SerializationNodeType::Object);
            }


            /// <summary>Returns mutable access to the archive serialization policy.</summary>
            SerializationPolicy& Policy() { return _policy; }
            /// <summary>Returns the archive serialization policy.</summary>
            const SerializationPolicy& Policy() const { return _policy; }

            /// <summary>Writes a declared serializable property while honoring its sensitivity metadata.</summary>
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

            /// <summary>Returns mutable access to the root serialization node.</summary>
            SerializationNode& GetNode() { return _root; }
            /// <summary>Returns the root serialization node.</summary>
            const SerializationNode& GetNode() const { return _root; }
            /// <summary>Returns mutable access to the root serialization node.</summary>
            SerializationNode& GetRootNode() { return _root; }
            /// <summary>Returns the root serialization node.</summary>
            const SerializationNode& GetRootNode() const { return _root; }

            /// <summary>Resets the archive to an empty object-rooted tree.</summary>
            void Clear() {
                _root = SerializationNode(SerializationNodeType::Object);
            }

            /// <summary>Writes a named value by converting it to the common serialization-node representation.</summary>
            template<typename TValue>
            void Write(const char* name, const TValue& value) {
                _root.Set(name, Detail::ToNode(value));
            }

            /// <summary>Determines whether a named root property exists.</summary>
            bool Contains(const char* name) const {
                return _root.Find(name) != nullptr;
            }

            /// <summary>Attempts to read and convert a named root property.</summary>
            /// <returns><c>true</c> when the property exists and can be converted to the requested type.</returns>
            template<typename TValue>
            bool Read(const char* name, TValue& value) {
                SerializationNode* child = _root.Find(name);
                return child != nullptr && Detail::FromNode(*child, value);
            }

            /// <summary>Reads a named property and returns detailed conversion and validation issues.</summary>
            template<typename TValue>
            DeserializationResult ReadDetailed(
                const char* name,
                TValue& value,
                const DeserializationOptions& options = {}
            ) {
                SerializationNode* child = _root.Find(name);
                if (child == nullptr) return {};
                return Detail::FromNodeDetailed(*child, value, name == nullptr ? "" : name, options);
            }
    };

}
