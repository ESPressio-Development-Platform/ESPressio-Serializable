#pragma once

#include <functional>
#include <shared_mutex>

#include "ESPressio_Property.hpp"


namespace ESPressio {

    namespace Serializable {

        template <typename T>
        class PropertyThreadSafe : public Property<T> {
            protected:
                using TOnValueChanged = std::function<void(T oldValue, T newValue)>;

                std::shared_mutex _valueMutex;
                std::shared_mutex _defaultValueMutex;
                std::shared_mutex _onValueChangedMutex;
                std::shared_mutex _onDefaultValueChangedMutex;

                virtual T DoGet() override {
                    std::shared_lock<std::shared_mutex> lock(_valueMutex);
                    return Property<T>::DoGet();
                }

                virtual T DoGetDefault() override {
                    std::shared_lock<std::shared_mutex> lock(_defaultValueMutex);
                    return Property<T>::DoGetDefault();
                }

                TOnValueChanged DoGetOnValueChanged() override {
                    std::shared_lock<std::shared_mutex> lock(_onValueChangedMutex);
                    return Property<T>::DoGetOnValueChanged();
                }

                TOnValueChanged DoGetOnDefaultValueChanged() override {
                    std::shared_lock<std::shared_mutex> lock(_onDefaultValueChangedMutex);
                    return Property<T>::DoGetOnDefaultValueChanged();
                }

                virtual void DoSet(T value) override {
                    std::unique_lock<std::shared_mutex> lock(_valueMutex);
                    Property<T>::DoSet(value);
                }

                virtual void DoSetDefault(T defaultValue) override {
                    std::unique_lock<std::shared_mutex> lock(_defaultValueMutex);
                    Property<T>::DoSetDefault(defaultValue);
                }

                void DoSetOnValueChanged(TOnValueChanged onValueChanged) override {
                    std::unique_lock<std::shared_mutex> lock(_onValueChangedMutex);
                    Property<T>::DoSetOnValueChanged(onValueChanged);
                }

                void DoSetOnDefaultValueChanged(TOnValueChanged onDefaultValueChanged) override {
                    std::unique_lock<std::shared_mutex> lock(_onDefaultValueChangedMutex);
                    Property<T>::DoSetOnDefaultValueChanged(onDefaultValueChanged);
                }

            public:
                PropertyThreadSafe(const char* name, T value, T defaultValue, TOnValueChanged onValueChanged = nullptr) : Property<T>(name, value, defaultValue, onValueChanged) { }

                PropertyThreadSafe(const char* name, T value, TOnValueChanged onValueChanged = nullptr) : Property<T>(name, value, onValueChanged) { }

                ~PropertyThreadSafe() {
                    std::unique_lock<std::shared_mutex> lockValue(_valueMutex);
                    std::unique_lock<std::shared_mutex> lockDefaultValue(_defaultValueMutex);
                    std::unique_lock<std::shared_mutex> lockOnValueChanged(_onValueChangedMutex);
                    std::unique_lock<std::shared_mutex> lockOnDefaultValueChanged(_onDefaultValueChangedMutex);
                }
        };

    }

}