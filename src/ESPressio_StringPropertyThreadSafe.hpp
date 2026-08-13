#pragma once

#include <functional>
#include <cstdlib>
#include <shared_mutex>

#include "ESPressio_StringProperty.hpp"

namespace ESPressio {

    namespace Serializable {

        class StringPropertyThreadSafe : public StringProperty {
            protected:
                using TOnValueChanged = std::function<void(const char* oldValue, const char* newValue)>;

                std::shared_mutex _valueMutex;
                std::shared_mutex _defaultValueMutex;
                std::shared_mutex _onValueChangedMutex;
                std::shared_mutex _onDefaultValueChangedMutex;

                const char* DoGet() override {
                    std::shared_lock<std::shared_mutex> lock(_valueMutex);
                    return StringProperty::DoGet();
                }

                const char* DoGetDefault() override {
                    std::shared_lock<std::shared_mutex> lock(_defaultValueMutex);
                    return StringProperty::DoGetDefault();
                }

                TOnValueChanged DoGetOnValueChanged() override {
                    std::shared_lock<std::shared_mutex> lock(_onValueChangedMutex);
                    return StringProperty::DoGetOnValueChanged();
                }

                TOnValueChanged DoGetOnDefaultValueChanged() override {
                    std::shared_lock<std::shared_mutex> lock(_onDefaultValueChangedMutex);
                    return StringProperty::DoGetOnDefaultValueChanged();
                }

                const char* DoExchangeValue(const char* value) override {
                    std::unique_lock<std::shared_mutex> lock(_valueMutex);
                    return StringProperty::DoExchangeValue(value);
                }

                const char* DoExchangeDefaultValue(const char* defaultValue) override {
                    std::unique_lock<std::shared_mutex> lock(_defaultValueMutex);
                    return StringProperty::DoExchangeDefaultValue(defaultValue);
                }

                void DoSetOnValueChanged(TOnValueChanged onValueChanged) override {
                    std::unique_lock<std::shared_mutex> lock(_onValueChangedMutex);
                    StringProperty::DoSetOnValueChanged(onValueChanged);
                }

                void DoSetOnDefaultValueChanged(TOnValueChanged onDefaultValueChanged) override {
                    std::unique_lock<std::shared_mutex> lock(_onDefaultValueChangedMutex);
                    StringProperty::DoSetOnDefaultValueChanged(onDefaultValueChanged);
                }
            public:
                StringPropertyThreadSafe(const char* name, const char* value, const char* defaultValue, TOnValueChanged onValueChanged = nullptr) : StringProperty(name, value, defaultValue, onValueChanged) { }

                StringPropertyThreadSafe(const char* name, const char* value, TOnValueChanged onValueChanged = nullptr) : StringProperty(name, value, onValueChanged) { }

                ~StringPropertyThreadSafe() { }
        };

    }

}
