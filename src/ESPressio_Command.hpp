#pragma once

#include <functional>

#include <ArduinoJson.h>

#include "ESPressio_ICommand.hpp"

namespace ESPressio {

    namespace Serializable{

        class Command : public ICommand {
            private:
                const char* _name;
                std::function<void(JsonObject&)> _callback;
            public:
                Command(const char* name, std::function<void(JsonObject&)> callback) : _name(strdup(name)), _callback(callback) { }

                ~Command() {
                    free((void*)_name);
                }

            // Methods

                void Call(JsonObject& parameters) override {
                    if (_callback == nullptr) { return; }
                    _callback(parameters);
                }
            
            // Getters

                const char* GetName() override {
                    return _name;
                }

                std::function<void(JsonObject&)> GetCallback() override {
                    return _callback;
                }

            // Setters

                void SetName(const char* name) override {
                    if (strcmp(_name, name) == 0) { return; }
                    free((void*)_name);
                    _name = strdup(name);
                }

                void SetCallback(std::function<void(JsonObject&)> callback) override {
                    _callback = callback;
                }
        };

    }

}