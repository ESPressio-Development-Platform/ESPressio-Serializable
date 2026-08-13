#pragma once

#include <functional>
#include <string>

#include <ArduinoJson.h>

#include "ESPressio_ICommand.hpp"

namespace ESPressio {

    namespace Serializable{

        class Command : public ICommand {
            private:
                std::string _name;
                std::function<void(JsonObject&)> _callback;
            protected:
            // Methods

                virtual void DoCall(JsonObject& parameters) {
                    if (_callback == nullptr) { return; }
                    _callback(parameters);
                }

            // Getters

                virtual std::function<void(JsonObject&)> DoGetCallback() {
                    return _callback;
                }

            // Setters

                virtual void DoSetCallback(std::function<void(JsonObject&)> callback) {
                    _callback = callback;
                }
            public:
                Command(const char* name, std::function<void(JsonObject&)> callback) : _name(name == nullptr ? "" : name), _callback(callback) { }

                Command(const Command&) = delete;
                Command& operator=(const Command&) = delete;
                Command(Command&&) = delete;
                Command& operator=(Command&&) = delete;

                ~Command() = default;

            // Methods

                inline void Call(JsonObject& parameters) override {
                    DoCall(parameters);
                }
            
            // Getters

                inline const char* GetName() override {
                    return _name.c_str();
                }

                inline std::function<void(JsonObject&)> GetCallback() override {
                    return DoGetCallback();
                }

            // Setters

                inline void SetCallback(std::function<void(JsonObject&)> callback) override {
                    DoSetCallback(callback);
                }
        };

    }

}
