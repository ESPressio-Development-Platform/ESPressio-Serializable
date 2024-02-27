#pragma once

#include <functional>

#include <ArduinoJson.h>

#include "ESPressio_IProperty.hpp"

namespace ESPressio {

    namespace Serializable {

            class IPropertyFactory {
            public:
                virtual void DoRegisterProperty(IProperty* property) = 0;
                virtual void RegisterProperties(std::initializer_list<IProperty*> properties) = 0;
                virtual void UnregisterProperty(IProperty* property) = 0;
                virtual void UnregisterProperty(const char* propertyName) = 0;
                virtual void ClearProperties() = 0;
                virtual IProperty* GetProperty(const char* propertyName) = 0;
                virtual bool HasProperty(const char* propertyName) = 0;
                virtual uint16_t WithProperties(std::function<void(IProperty*)> propertyFunction) = 0;
                virtual uint16_t WritePropertiesToJson(JsonArray& array) = 0;
                virtual uint16_t WritePropertiesToJson(JsonObject& object) = 0;
                virtual uint16_t WritePropertiesToJson(JsonDocument& document) = 0;
                virtual uint16_t WriteDefaultPropertiesToJson(JsonArray& array) = 0;
                virtual uint16_t WriteDefaultPropertiesToJson(JsonObject& object) = 0;
                virtual uint16_t WriteDefaultPropertiesToJson(JsonDocument& document) = 0;
                virtual uint16_t ReadPropertiesFromJson(JsonObject& jsonObject, bool useDefaultValue = false) = 0;
                virtual uint16_t ReadPropertiesFromJson(JsonDocument& jsonDocument, bool useDefaultValue = false) = 0;
        };

    }

}