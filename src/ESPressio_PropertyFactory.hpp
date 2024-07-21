#pragma once

#include <functional>
#include <unordered_map>

#include <ArduinoJson.h>

#include "ESPressio_IProperty.hpp"
#include "ESPressio_IPropertyFactory.hpp"
#include "ESPressio_ISerializable.hpp"
#include "ESPressio_SerializableBase.hpp"

namespace ESPressio {

    namespace Serializable {

        class PropertyFactory : public IPropertyFactory, public SerializableBase {
            private:
                std::unordered_map<std::string, IProperty*> _properties;
            protected:
                virtual IProperty* DoGetProperty(const char* propertyName) {
                    return _properties[std::string(propertyName)];
                }

                virtual void DoRegisterProperty(IProperty* property) {
                    _properties[std::string(property->GetName())] = property;
                }

                virtual void DoRegisterProperties(std::initializer_list<IProperty*> properties) {
                    for (auto property : properties) {
                        _properties[std::string(property->GetName())] = property;
                    }
                }

                virtual void DoUnregisterProperty(IProperty* property) {
                    _properties.erase(std::string(property->GetName()));
                }

                virtual void DoUnregisterProperty(const char* propertyName) {
                    _properties.erase(std::string(propertyName));
                }

                virtual void DoClearProperties() {
                    _properties.clear();
                }

                virtual uint16_t DoWithProperties(std::function<void(IProperty*)> propertyFunction) {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        propertyFunction(property.second);
                    }

                    return count;
                }

                virtual uint16_t DoWritePropertiesToJson(JsonArray& array) {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteToJson(array);
                    }

                    return count;
                }

                virtual uint16_t DoWritePropertiesToJson(JsonObject& object) {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteToJson(object);
                    }

                    return count;
                }

                virtual uint16_t DoWritePropertiesToJson(JsonDocument& document) {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteToJson(document);
                    }

                    return count;
                }

                virtual uint16_t DoWriteDefaultPropertiesToJson(JsonArray& array) {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteDefaultToJson(array);
                    }

                    return count;
                }

                virtual uint16_t DoWriteDefaultPropertiesToJson(JsonObject& object) {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteDefaultToJson(object);
                    }

                    return count;
                }

                virtual uint16_t DoWriteDefaultPropertiesToJson(JsonDocument& document) {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteDefaultToJson(document);
                    }

                    return count;
                }

                virtual uint16_t DoReadPropertiesFromJson(JsonObject& jsonObject, bool useDefaultValue = false) {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->ReadValueFromJson(jsonObject, useDefaultValue);
                    }

                    return count;
                }

                virtual uint16_t DoReadPropertiesFromJson(JsonDocument& jsonDocument, bool useDefaultValue = false) {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->ReadValueFromJson(jsonDocument, useDefaultValue);
                    }

                    return count;
                }

            // SerializableBase Overrides

                virtual bool DoDeSerializeNonProperty(JsonVariant& jsonVariant, IDeserializeResults* results = nullptr) {
                    if (results != nullptr) {
                        //TODO: Add implementation to use for DeserializeResult
                        // results->AddResult(new DeserializeResult(jsonVariant.key().c_str(), DeserializeResultType::SUCCESS));
                    }
                    return false;
                }

                virtual bool DoDeserialize(JsonObject& jsonObject, IDeserializeResults* results = nullptr) override {
                    bool success = true;

                    for (auto keyValuePair : jsonObject) {
                        IProperty* property = GetProperty(keyValuePair.key().c_str());
                        if (property != nullptr) {
                            property->ReadValueFromJson(jsonObject, false);
                            //TODO: Add implementation to use for DeserializeResult
                            // results->AddResult(new DeserializeResult(keyValuePair.key().c_str(), DeserializeResultType::SUCCESS));
                        }
                        else if (!DoDeSerializeNonProperty(keyValuePair, results)) {
                            success = false;
                        }
                    }

                    return success;
                }

                virtual bool DoValidateJson(JsonObject& jsonObject, IDeserializeResults* results = nullptr) override {

                }

                virtual void DoSerialize(JsonObject& jsonObject) override {
                    WritePropertiesToJson(jsonObject);
                }
            public:
                PropertyFactory() {

                }

                inline IProperty* GetProperty(const char* propertyName) override {
                    return DoGetProperty(propertyName);
                }


                inline bool HasProperty(const char* propertyName) override {
                    return GetProperty(propertyName) != nullptr;
                }

                inline void RegisterProperty(IProperty* property) {
                    DoRegisterProperty(property);
                }

                inline void RegisterProperties(std::initializer_list<IProperty*> properties) {
                    DoRegisterProperties(properties);
                }

                inline void UnregisterProperty(IProperty* property) {
                    DoUnregisterProperty(property);
                }

                inline void UnregisterProperty(const char* propertyName) {
                    DoUnregisterProperty(propertyName);
                }

                inline void ClearProperties() {
                    DoClearProperties();
                }

                inline uint16_t WithProperties(std::function<void(IProperty*)> propertyFunction) override {
                    return DoWithProperties(propertyFunction);
                }

                inline uint16_t WritePropertiesToJson(JsonArray& array) override {
                    return DoWritePropertiesToJson(array);
                }

                inline uint16_t WritePropertiesToJson(JsonObject& object) override {
                    return DoWritePropertiesToJson(object);
                }

                inline uint16_t WritePropertiesToJson(JsonDocument& document) override {
                    return DoWritePropertiesToJson(document);
                }

                inline uint16_t WriteDefaultPropertiesToJson(JsonArray& array) override {
                    return DoWriteDefaultPropertiesToJson(array);
                }

                inline uint16_t WriteDefaultPropertiesToJson(JsonObject& object) override {
                    return DoWriteDefaultPropertiesToJson(object);
                }

                inline uint16_t WriteDefaultPropertiesToJson(JsonDocument& document) override {
                    return DoWriteDefaultPropertiesToJson(document);
                }

                inline uint16_t ReadPropertiesFromJson(JsonObject& jsonObject, bool useDefaultValue = false) override {
                    return DoReadPropertiesFromJson(jsonObject, useDefaultValue);
                }

                inline uint16_t ReadPropertiesFromJson(JsonDocument& jsonDocument, bool useDefaultValue = false) override {
                    return DoReadPropertiesFromJson(jsonDocument, useDefaultValue);
                }
        };

    }

}