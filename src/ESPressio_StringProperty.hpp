#pragma once

#include <functional>

#include <ArduinoJson.h>

#include "ESPressio_Property.hpp"

namespace ESPressio {

    namespace Serializable {

        class StringProperty : public Property<const char*> {
            protected:
                inline bool DoCompareEqual(const char* a, const char* b) override {
                    return strcmp(a, b) == 0;
                }

                void DoSet(const char* value) override {
                    free((void*)_value); // We must destroy the old value before setting the new one
                    _value = strdup(value);
                }

                void DoSetDefault(const char* defaultValue) override {
                    free((void*)_defaultValue); // We must destroy the old value before setting the new one
                    _defaultValue = strdup(defaultValue);
                }
            public:
                StringProperty(const char* name, const char* value, const char* defaultValue, TOnValueChanged onValueChanged = nullptr) : Property<const char*>(name, strdup(value), strdup(defaultValue), onValueChanged) { }

                StringProperty(const char* name, const char* value, TOnValueChanged onValueChanged = nullptr) : Property<const char*>(name, strdup(value), onValueChanged) { }

                ~StringProperty() {
                    free((void*)_value);
                    free((void*)_defaultValue);
                }

                void SetValue(const char* value)  override {
                    const char* newVal = strdup(value);
                    const char* oldValue = _value;
                    if (DoCompareEqual(oldValue, newVal)) {
                        free((void*)newVal);
                        return;
                    }
                    DoSet(newVal);
                    if (_onValueChanged != nullptr) { _onValueChanged(oldValue, _value); }
                }
        };

    }

}