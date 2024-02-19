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
                std::unordered_map<const char*, IProperty*> _properties;
            protected:
                void RegisterProperty(IProperty* property) {
                    _properties[strdup(property->GetName())] = property;
                }

                void RegisterProperties(std::initializer_list<IProperty*> properties) {
                    for (auto property : properties) {
                        _properties[property->GetName()] = property;
                    }
                }

                void UnregisterProperty(IProperty* property) {
                    _properties.erase(property->GetName());
                }

                void UnregisterProperty(const char* propertyName) {
                    _properties.erase(propertyName);
                }

                void ClearProperties() {
                    _properties.clear();
                }
            public:
                PropertyFactory() {

                }

                IProperty* GetProperty(const char* propertyName) {
                    return _properties[propertyName];
                }


                bool HasProperty(const char* propertyName) {
                    // return _properties.find(propertyName) != _properties.end();
                    return GetProperty(propertyName) != nullptr;
                }

                uint16_t WithProperties(std::function<void(IProperty*)> propertyFunction) {
                    std::unordered_map<const char*, IProperty*> properties = _properties;

                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        propertyFunction(property.second);
                    }

                    return count;
                }

                uint16_t WritePropertiesToJson(JsonArray& array) {
                    std::unordered_map<const char*, IProperty*> properties = _properties;

                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteToJson(array);
                    }

                    return count;
                }

                uint16_t WritePropertiesToJson(JsonObject& object) {
                    std::unordered_map<const char*, IProperty*> properties = _properties;

                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteToJson(object);
                    }

                    return count;
                }

                uint16_t WritePropertiesToJson(JsonDocument& document) {
                    std::unordered_map<const char*, IProperty*> properties = _properties;

                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteToJson(document);
                    }

                    return count;
                }

                uint16_t WriteDefaultPropertiesToJson(JsonArray& array) {
                    std::unordered_map<const char*, IProperty*> properties = _properties;

                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteDefaultToJson(array);
                    }

                    return count;
                }

                uint16_t WriteDefaultPropertiesToJson(JsonObject& object) {
                    std::unordered_map<const char*, IProperty*> properties = _properties;

                    uint32_t count = 0;

                    for (auto property : _properties) {
                        property.second->WriteDefaultToJson(object);
                        count++;
                    }

                    return count;
                }

                uint16_t WriteDefaultPropertiesToJson(JsonDocument& document) {
                    std::unordered_map<const char*, IProperty*> properties = _properties;

                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->WriteDefaultToJson(document);
                    }

                    return count;
                }

                uint16_t ReadPropertiesFromJson(JsonObject& jsonObject, bool useDefaultValue = false) {
                    std::unordered_map<const char*, IProperty*> properties = _properties;

                    uint32_t count = 0;

                    for (auto property : _properties) {
                        count++;
                        property.second->ReadValueFromJson(jsonObject, useDefaultValue);
                    }

                    return count;
                }

                uint16_t ReadPropertiesFromJson(JsonDocument& jsonDocument, bool useDefaultValue = false) {
                    std::unordered_map<const char*, IProperty*> properties = _properties;

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