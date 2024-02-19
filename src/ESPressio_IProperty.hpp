#pragma once

#include <ArduinoJson.h>

namespace ESPressio {

    namespace Serializable {

        class IProperty {
            protected:
                const char* _name;
            public:
                IProperty(const char* name) : _name(name) {

                }

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

    }

}