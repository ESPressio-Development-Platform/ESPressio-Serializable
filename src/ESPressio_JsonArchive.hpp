#pragma once

#if !__has_include(<ArduinoJson.h>)
    #error "ESPressio_JsonArchive.hpp requires ArduinoJson. Add bblanchon/ArduinoJson to your project's lib_deps, or do not include this optional adapter."
#endif

#include <ArduinoJson.h>
#include <string>

#include "ESPressio_TreeArchive.hpp"

namespace ESPressio::Serializable {

    class JsonArchive : public TreeArchive {
        private:
            bool _valid = true;

            static void NodeToJson(
                const SerializationNode& node,
                ArduinoJson::JsonVariant output
            ) {
                switch (node.GetType()) {
                    case SerializationNodeType::Null:
                        output.set(nullptr);
                        break;

                    case SerializationNodeType::Object: {
                        ArduinoJson::JsonObject object =
                            output.to<
                                ArduinoJson::JsonObject
                            >();

                        for (
                            const auto& child :
                            node.ObjectChildren()
                        ) {
                            NodeToJson(
                                child.second,
                                object[
                                    child.first.c_str()
                                ]
                            );
                        }
                        break;
                    }

                    case SerializationNodeType::Array: {
                        ArduinoJson::JsonArray array =
                            output.to<
                                ArduinoJson::JsonArray
                            >();

                        for (
                            const auto& child :
                            node.ArrayChildren()
                        ) {
                            ArduinoJson::JsonVariant item =
                                array.add<
                                    ArduinoJson::JsonVariant
                                >();

                            NodeToJson(
                                child,
                                item
                            );
                        }
                        break;
                    }

                    case SerializationNodeType::Boolean:
                        output.set(
                            node.BooleanValue()
                        );
                        break;

                    case SerializationNodeType::SignedInteger:
                        output.set(
                            node.SignedIntegerValue()
                        );
                        break;

                    case SerializationNodeType::UnsignedInteger:
                        output.set(
                            node.UnsignedIntegerValue()
                        );
                        break;

                    case SerializationNodeType::Float32:
                        output.set(
                            node.Float32Value()
                        );
                        break;

                    case SerializationNodeType::Float64:
                        output.set(
                            node.Float64Value()
                        );
                        break;

                    case SerializationNodeType::String:
                        output.set(
                            node.StringValue().c_str()
                        );
                        break;
                }
            }

            static bool JsonToNode(
                ArduinoJson::JsonVariantConst input,
                SerializationNode& node
            ) {
                if (input.isNull()) {
                    node.SetType(
                        SerializationNodeType::Null
                    );

                    return true;
                }

                if (
                    input.is<
                        ArduinoJson::JsonObjectConst
                    >()
                ) {
                    const auto object =
                        input.as<
                            ArduinoJson::JsonObjectConst
                        >();

                    node.ReserveObject(
                        object.size()
                    );

                    for (
                        ArduinoJson::JsonPairConst pair :
                        object
                    ) {
                        SerializationNode child;

                        if (
                            !JsonToNode(
                                pair.value(),
                                child
                            )
                        ) {
                            return false;
                        }

                        node.Set(
                            pair.key().c_str(),
                            std::move(child)
                        );
                    }

                    return true;
                }

                if (
                    input.is<
                        ArduinoJson::JsonArrayConst
                    >()
                ) {
                    const auto array =
                        input.as<
                            ArduinoJson::JsonArrayConst
                        >();

                    node.ReserveArray(
                        array.size()
                    );

                    for (
                        ArduinoJson::JsonVariantConst value :
                        array
                    ) {
                        SerializationNode child;

                        if (
                            !JsonToNode(
                                value,
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
                }

                if (input.is<bool>()) {
                    node.SetType(
                        SerializationNodeType::Boolean
                    );

                    node.BooleanValue() =
                        input.as<bool>();

                    return true;
                }

                if (input.is<int64_t>()) {
                    node.SetType(
                        SerializationNodeType::SignedInteger
                    );

                    node.SignedIntegerValue() =
                        input.as<int64_t>();

                    return true;
                }

                if (input.is<uint64_t>()) {
                    node.SetType(
                        SerializationNodeType::UnsignedInteger
                    );

                    node.UnsignedIntegerValue() =
                        input.as<uint64_t>();

                    return true;
                }

                if (input.is<double>()) {
                    node.SetType(
                        SerializationNodeType::Float64
                    );

                    node.Float64Value() =
                        input.as<double>();

                    return true;
                }

                if (input.is<const char*>()) {
                    node.SetType(
                        SerializationNodeType::String
                    );

                    const char* text =
                        input.as<const char*>();

                    if (text == nullptr) {
                        node.StringValue().clear();
                    } else {
                        node.StringValue().assign(text);
                    }

                    return true;
                }

                return false;
            }

            ArduinoJson::JsonDocument MakeDocument() const {
                ArduinoJson::JsonDocument document;

                ArduinoJson::JsonVariant root =
                    document.to<
                        ArduinoJson::JsonVariant
                    >();

                NodeToJson(
                    _root,
                    root
                );

                return document;
            }

        public:
            bool LoadDocument(const ArduinoJson::JsonDocument& document) {
                SerializationNode root;
                _valid = JsonToNode(document.as<ArduinoJson::JsonVariantConst>(), root) && root.GetType() == SerializationNodeType::Object;
                if (_valid) _root = std::move(root);
                return _valid;
            }

            JsonArchive() = default;

            template<typename TInput>
            explicit JsonArchive(TInput& input) {
                Load(input);
            }

            bool IsValid() const {
                return _valid;
            }

            template<typename TOutput>
            size_t Save(TOutput& output) const {
                auto document =
                    MakeDocument();

                return ArduinoJson::serializeJson(
                    document,
                    output
                );
            }

            template<typename TOutput>
            size_t SavePretty(TOutput& output) const {
                auto document =
                    MakeDocument();

                return ArduinoJson::serializeJsonPretty(
                    document,
                    output
                );
            }

            std::string ToString() const {
                std::string output;
                auto document =
                    MakeDocument();

                ArduinoJson::serializeJson(
                    document,
                    output
                );

                return output;
            }

            template<typename TInput>
            bool Load(TInput& input) {
                ArduinoJson::JsonDocument document;

                const auto error =
                    ArduinoJson::deserializeJson(
                        document,
                        input
                    );

                if (error) {
                    _valid = false;
                    return false;
                }

                SerializationNode root;

                _valid =
                    JsonToNode(
                        document.as<
                            ArduinoJson::JsonVariantConst
                        >(),
                        root
                    ) &&
                    root.GetType() ==
                        SerializationNodeType::Object;

                if (_valid) {
                    _root = std::move(root);
                }

                return _valid;
            }

            bool Load(const char* input) {
                ArduinoJson::JsonDocument document;

                const auto error =
                    ArduinoJson::deserializeJson(
                        document,
                        input
                    );

                if (error) {
                    _valid = false;
                    return false;
                }

                SerializationNode root;

                _valid =
                    JsonToNode(
                        document.as<
                            ArduinoJson::JsonVariantConst
                        >(),
                        root
                    ) &&
                    root.GetType() ==
                        SerializationNodeType::Object;

                if (_valid) {
                    _root = std::move(root);
                }

                return _valid;
            }

            bool Load(const std::string& input) {
                return Load(
                    input.c_str()
                );
            }
    };

}
