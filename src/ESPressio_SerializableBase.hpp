#pragma once

#include <ArduinoJson.h>

#include "ESPressio_ISerializable.hpp"

namespace ESPressio {

    namespace Serializable {

        class SerializableBase : public ISerializable {
            protected:
                virtual bool DoDeserialize(JsonObject& jsonObject, IDeserializeResults* results = nullptr) = 0;
                virtual bool DoValidateJson(JsonObject& jsonObject, IDeserializeResults* results = nullptr) = 0;
                virtual void DoSerialize(JsonObject& jsonObject) = 0;
            public:
                SerializableBase() = default;

                SerializableBase(JsonObject& jsonObject, IDeserializeResults* results = nullptr) {
                    DoDeserialize(jsonObject, results);
                }

                inline bool Deserialize(JsonObject& jsonObject, IDeserializeResults* results = nullptr) override {
                    return DoDeserialize(jsonObject, results);
                }

                inline bool ValidateJson(JsonObject& jsonObject, IDeserializeResults* results = nullptr) override {
                    return DoValidateJson(jsonObject, results);
                }

                inline void Serialize(JsonObject& jsonObject) override {
                    DoSerialize(jsonObject);
                }

        };

    }

}