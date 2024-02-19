#pragma once

#include <functional>

#include <ArduinoJson.h>

#include "ESPressio_Property.hpp"

namespace ESPressio {

    namespace Serializable {

        class StringProperty : public Property<const char*> {
            protected:
                bool DoCompareEqual(T a, T b) override {
                    return !strcmp(a, b) == 0;
                }

                void DoSet(T value) override {
                    free((void*)_value); // We must destroy the old value before setting the new one
                    _value = value;
                }

                void DoSetDefault(T defaultValue) override {
                    free((void*)_defaultValue); // We must destroy the old value before setting the new one
                    _defaultValue = defaultValue;
                }
            public:
                StringProperty(const char* name, const char* value, const char* defaultValue, TOnValueChanged onValueChanged = nullptr) : Property<const char*>(name, strdup(value), strdup(defaultValue), onValueChanged) { }

                StringProperty(const char* name, const char* value, TOnValueChanged onValueChanged = nullptr) : Property<const char*>(name, strdup(value), onValueChanged) { }

                ~StringProperty() {
                    free((void*)_value);
                    free((void*)_defaultValue);
                }
        };

    }

}