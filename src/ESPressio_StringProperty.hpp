#pragma once

#include <functional>
#include <cstdlib>
#include <cstring>

#include "ESPressio_Property.hpp"

namespace ESPressio {

    namespace Serializable {

        class StringProperty : public Property<const char*> {
            protected:
                using TOnValueChanged = std::function<void(const char* oldValue, const char* newValue)>;

                inline bool DoCompareEqual(const char* a, const char* b) override {
                    if (a == b) { return true; }
                    if (a == nullptr || b == nullptr) { return false; }
                    return strcmp(a, b) == 0;
                }

                static const char* Duplicate(const char* value) {
                    return value == nullptr ? nullptr : strdup(value);
                }

                virtual const char* DoExchangeValue(const char* value) {
                    const char* oldValue = _value;
                    Property<const char*>::DoSet(value);
                    return oldValue;
                }

                virtual const char* DoExchangeDefaultValue(const char* defaultValue) {
                    const char* oldDefaultValue = _defaultValue;
                    Property<const char*>::DoSetDefault(defaultValue);
                    return oldDefaultValue;
                }
            public:
                StringProperty(const char* name, const char* value, const char* defaultValue, TOnValueChanged onValueChanged = nullptr) : Property<const char*>(name, nullptr, nullptr, onValueChanged) {
                    _value = Duplicate(value);
                    _defaultValue = Duplicate(defaultValue);
                }

                StringProperty(const char* name, const char* value, TOnValueChanged onValueChanged = nullptr) : StringProperty(name, value, value, onValueChanged) { }

                ~StringProperty() {
                    free((void*)_value);
                    free((void*)_defaultValue);
                }

                void SetValue(const char* value) override {
                    const char* currentValue = GetValue();
                    if (DoCompareEqual(currentValue, value)) { return; }

                    const char* newValue = Duplicate(value);
                    if (value != nullptr && newValue == nullptr) { return; }

                    const char* oldValue = DoExchangeValue(newValue);
                    TOnValueChanged onValueChanged = GetOnValueChanged();
                    if (onValueChanged != nullptr) {
                        onValueChanged(oldValue, newValue);
                    }
                    free((void*)oldValue);
                }

                void SetDefaultValue(const char* defaultValue) override {
                    const char* currentDefaultValue = GetDefaultValue();
                    if (DoCompareEqual(currentDefaultValue, defaultValue)) { return; }

                    const char* newDefaultValue = Duplicate(defaultValue);
                    if (defaultValue != nullptr && newDefaultValue == nullptr) { return; }

                    const char* oldDefaultValue = DoExchangeDefaultValue(newDefaultValue);
                    TOnValueChanged onDefaultValueChanged = GetOnDefaultValueChanged();
                    if (onDefaultValueChanged != nullptr) {
                        onDefaultValueChanged(oldDefaultValue, newDefaultValue);
                    }
                    free((void*)oldDefaultValue);
                }
        };

    }

}
