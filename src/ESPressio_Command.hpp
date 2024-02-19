#pragma once

#include <functional>

#include <ArduinoJson.h>

#include "ESPressio_ICommand.hpp"

namespace ESPressio {

    namespace Serializable{

        class Command : public ICommand {
            private:
                char* _name;
                std::function<void(JsonObject&)> _callback;
            public:
                Command(char* name, std::function<void(JsonObject&)> callback) : _name(strdup(name)), _callback(callback) { }

                ~Command() {
                    free((void*)_name);
                }

            // Methods

                void Call(JsonObject& parameters) override {
                    if (_callback == nullptr) { return; }
                    _callback(parameters);
                }
            
            // Getters

                char* GetName() override {
                    return _name;
                }

                std::function<void(JsonObject&)> GetCallback() override {
                    return _callback;
                }

            // Setters

                void SetName(char* name) override {
                    free((void*)_name);
                    _name = strdup(name);
                }

                void SetCallback(std::function<void(JsonObject&)> callback) override {
                    _callback = callback;
                }
        };

    }

}