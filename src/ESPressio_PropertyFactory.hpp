#pragma once

#include <functional>
#include <unordered_map>

#include <ArduinoJson.h>

#include "ESPressio_IProperty.hpp"
#include "ESPressio_IPropertyFactory.hpp"

namespace ESPressio {

    namespace Serializable {

        class PropertyFactory : public IPropertyFactory {
            private:
                std::unordered_map<std::string, IProperty*> _properties;
            protected:
                void RegisterProperty(IProperty* property) {
                    _properties[std::string(property->GetName())] = property;
                }

                void RegisterProperties(std::initializer_list<IProperty*> properties) {
                    for (auto property : properties) {
                        _properties[std::string(property->GetName())] = property;
                    }
                }

                void UnregisterProperty(IProperty* property) {
                    _properties.erase(std::string(property->GetName()));
                }

                void UnregisterProperty(const char* propertyName) {
                    _properties.erase(std::string(propertyName));
                }

                void ClearProperties() {
                    _properties.clear();
                }
            public:
                PropertyFactory() {

                }

                IProperty* GetProperty(const char* propertyName) override {
                    return _properties[std::string(propertyName)];
                }


                bool HasProperty(const char* propertyName) override {
                    return GetProperty(propertyName) != nullptr;
                }

                uint16_t WithProperties(std::function<void(IProperty*)> propertyFunction) override {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        propertyFunction(property.second);
                    }

                    return count;
                }

                uint16_t WritePropertiesToJson(JsonArray& array) override {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteToJson(array);
                    }

                    return count;
                }

                uint16_t WritePropertiesToJson(JsonObject& object) override {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteToJson(object);
                    }

                    return count;
                }

                uint16_t WritePropertiesToJson(JsonDocument& document) override {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteToJson(document);
                    }

                    return count;
                }

                uint16_t WriteDefaultPropertiesToJson(JsonArray& array) override {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteDefaultToJson(array);
                    }

                    return count;
                }

                uint16_t WriteDefaultPropertiesToJson(JsonObject& object) override {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        property.second->WriteDefaultToJson(object);
                        count++;
                    }

                    return count;
                }

                uint16_t WriteDefaultPropertiesToJson(JsonDocument& document) override {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteDefaultToJson(document);
                    }

                    return count;
                }

                uint16_t ReadPropertiesFromJson(JsonObject& jsonObject, bool useDefaultValue = false) override {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->ReadValueFromJson(jsonObject, useDefaultValue);
                    }

                    return count;
                }

                uint16_t ReadPropertiesFromJson(JsonDocument& jsonDocument, bool useDefaultValue = false) override {
                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->ReadValueFromJson(jsonDocument, useDefaultValue);
                    }

                    return count;
                }
        };

    }

}