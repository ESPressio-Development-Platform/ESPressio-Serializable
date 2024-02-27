#pragma once

#include <ArduinoJson.h>
#include <cstdlib>
#include <cstring>
#include <functional>

namespace ESPressio {

    namespace Serializable {

        class IProperty {
            public:
                virtual ~IProperty() = default;
                virtual void WriteToJson(JsonArray& array) = 0;
                virtual void WriteToJson(JsonObject& object) = 0;
                virtual void WriteToJson(JsonDocument& document) = 0;
                virtual void WriteDefaultToJson(JsonArray& array) = 0;
                virtual void WriteDefaultToJson(JsonObject& object) = 0;
                virtual void WriteDefaultToJson(JsonDocument& document) = 0;
                virtual void ReadValueFromJson(JsonObject& jsonObject, bool useDefaultValue = false) = 0;
                virtual void ReadValueFromJson(JsonDocument& jsonDocument, bool useDefaultValue = false) = 0;
                virtual void ResetToDefault() = 0;
                virtual const char* GetName() = 0;
        };

        template <typename T>
        class ITypedProperty : public IProperty {
            public:
                using TOnValueChanged = std::function<void(T oldValue, T newValue)>;
                
            // Getters
                
                virtual T GetValue() = 0;
                virtual T GetDefaultValue() = 0;
                virtual TOnValueChanged GetOnValueChanged() = 0;
                virtual TOnValueChanged GetOnDefaultValueChanged() = 0;

            // Setters

                virtual void SetValue(T value) = 0;
                virtual void SetDefaultValue(T defaultValue) = 0;
                virtual void SetOnValueChanged(TOnValueChanged onValueChanged) = 0;
                virtual void SetOnDefaultValueChanged(TOnValueChanged onDefaultValueChanged) = 0;

        };

    }

}