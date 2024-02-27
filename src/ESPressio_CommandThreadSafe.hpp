#pragma once

#include <functional>
#include <mutex>
#include <shared_mutex>

#include <ArduinoJson.h>

#include "ESPressio_Command.hpp"

namespace ESPressio {

    namespace Serializable{

        class CommandThreadSafe : public Command {
            private:
                std::shared_mutex _callbackMutex;
            protected:
            // Methods

                void DoCall(JsonObject& parameters) override {
                    std::unique_lock<std::shared_mutex> lock(_callbackMutex);
                    Command::DoCall(parameters);
                }

            // Getters

                std::function<void(JsonObject&)> DoGetCallback() override {
                    std::shared_lock<std::shared_mutex> lock(_callbackMutex);
                    return Command::DoGetCallback();
                }

            // Setters

                void DoSetCallback(std::function<void(JsonObject&)> callback) override {
                    std::unique_lock<std::shared_mutex> lock(_callbackMutex);
                    Command::DoSetCallback(callback);
                }
            public:
                CommandThreadSafe(const char* name, std::function<void(JsonObject&)> callback) : Command(name, callback) { }

                ~CommandThreadSafe() { 
                    std::unique_lock<std::shared_mutex> lock(_callbackMutex);
                }
        };

    }

}