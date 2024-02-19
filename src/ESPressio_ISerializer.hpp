#pragma once

#include <ArduinoJson.h>

#include "ESPressio_IProperty.hpp"
#include "ESPressio_IPropertyFactory.hpp"

namespace ESPressio {

    namespace Serializable {

        class ISerializer {
        public:
            virtual ~ISerializer() = default;

            virtual bool Deserialize(IPropertyFactory* propertyFactory, JsonDocument& jsonDocument) = 0;
            virtual bool Deserialize(IPropertyFactory* propertyFactory, JsonObject& jsonObject) = 0;
            virtual bool Deserialize(IPropertyFactory* propertyFactory, JsonArray& jsonArray) = 0;

            virtual bool Deserialize(IProperty* property, JsonDocument& jsonDocument) = 0;
            virtual bool Deserialize(IProperty* property, JsonObject& jsonObject) = 0;
            virtual bool Deserialize(IProperty* property, JsonArray& jsonArray) = 0;

            virtual bool Serialize(IPropertyFactory* propertyFactory, JsonDocument& jsonDocument) = 0;
            virtual bool Serialize(IPropertyFactory* propertyFactory, JsonObject& jsonObject) = 0;
            virtual bool Serialize(IPropertyFactory* propertyFactory, JsonArray& jsonArray) = 0;

            virtual bool Serialize(IProperty* property, JsonDocument& jsonDocument) = 0;
            virtual bool Serialize(IProperty* property, JsonObject& jsonObject) = 0;
            virtual bool Serialize(IProperty* property, JsonArray& jsonArray) = 0;
        };

    }

}