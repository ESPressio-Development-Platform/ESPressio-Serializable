#pragma once

#include <functional>

#include <ArduinoJson.h>

namespace ESPressio {

    namespace Serializable {

        class ICommand {
            public:
                // Methods
                virtual void Call(JsonObject& parameters) = 0;

                // Getters
                virtual char* GetName() = 0;
                virtual std::function<void(JsonObject&)> GetCallback() = 0;

                // Setters
                virtual void SetName(char* name) = 0;
                virtual void SetCallback(std::function<void(JsonObject&)> callback) = 0;
        };

    }

}