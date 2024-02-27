#pragma once

#include <functional>
#include <cstdlib>

#include "ESPressio_Property.hpp"

namespace ESPressio {

    namespace Serializable {

        class StringProperty : public Property<const char*> {
            protected:
                using TOnValueChanged = std::function<void(const char* oldValue, const char* newValue)>;

                inline bool DoCompareEqual(const char* a, const char* b) override {
                    return strcmp(a, b) == 0;
                }

                virtual void DoSet(const char* value) override {
                    free((void*)_value); // We must destroy the old value before setting the new one
                    Property<const char*>::DoSet(strdup(value));
                }

                virtual void DoSetDefault(const char* defaultValue) override {
                    free((void*)_defaultValue); // We must destroy the old value before setting the new one
                    Property<const char*>::DoSetDefault(strdup(defaultValue));
                }
            public:
                StringProperty(const char* name, const char* value, const char* defaultValue, TOnValueChanged onValueChanged = nullptr) : Property<const char*>(name, strdup(value), strdup(defaultValue), onValueChanged) { }

                StringProperty(const char* name, const char* value, TOnValueChanged onValueChanged = nullptr) : Property<const char*>(name, strdup(value), onValueChanged) { }

                ~StringProperty() {
                    free((void*)_value);
                    free((void*)_defaultValue);
                }

                void SetValue(const char* value) override {
                    Property<const char*>::SetValue(strdup(value));
                }
        };

    }

}