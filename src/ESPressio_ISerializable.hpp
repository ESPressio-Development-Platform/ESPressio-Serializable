#pragma once

#include <ArduinoJson.h>
#include <cstdint>
#include <vector>

namespace ESPressio {

    namespace Serializable {

        enum DeserializeResultType {
            SUCCESS,
            FAILURE
        };

        class IDeserializeResult {
            public:
                virtual ~IDeserializeResult() = default;

            // Getters

                virtual const char* GetFieldName() = 0;
                virtual DeserializeResultType GetResultType() = 0;
                
                inline bool IsSuccess() { return (GetResultType() == DeserializeResultType::SUCCESS ); }
                inline bool IsFailure() { return (GetResultType() == DeserializeResultType::FAILURE ); }
        };

        class IDeserializeResults {
            public:
                virtual ~IDeserializeResults() = default;

                virtual void AddResult(IDeserializeResult* result) = 0;
                virtual void ClearResults() = 0;

                virtual uint16_t GetCountSucceeded() = 0;
                virtual uint16_t GetCountFailed() = 0;
        };

        class ISerializable {
            public:
                virtual ~ISerializable() = default;

                virtual bool Deserialize(JsonObject& jsonObject, IDeserializeResults* results = nullptr) = 0;
                virtual bool ValidateJson(JsonObject& jsonObject, IDeserializeResults* results = nullptr) = 0;
                virtual void Serialize(JsonObject& jsonObject) = 0;
        };

    }

}