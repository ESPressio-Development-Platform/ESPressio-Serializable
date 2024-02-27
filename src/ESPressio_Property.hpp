#pragma once

#include <functional>

#include <ArduinoJson.h>

#include "ESPressio_IProperty.hpp"


namespace ESPressio {

    namespace Serializable {

        template <typename T>
        class Property : public IProperty {
            private:
                const char* _name; // Name is Idempotent by design
            protected:
                using TOnValueChanged = std::function<void(T oldValue, T newValue)>;
                volatile T _value;
                volatile T _defaultValue;
                TOnValueChanged _onValueChanged = nullptr; // Callback to notify the Parent when this Property's value changes
                TOnValueChanged _onDefaultValueChanged = nullptr; // Callback to notify the Parent when this Property's default value changes

                void AddPropertyValue(const char* propertyName, JsonArray& properties, T value) {
                    // Create a new JSON object
                    JsonObject property = properties.createNestedObject();
                    // Add the name of the property
                    property["name"] = propertyName;
                    // Add the value of the property
                    property["value"] = value;
                }

                virtual bool DoCompareEqual(T a, T b) {
                    return a == b;
                }

                virtual T DoGet() {
                    return _value;
                }

                virtual T DoGetDefault() {
                    return _defaultValue;
                }

                virtual TOnValueChanged DoGetOnValueChanged() {
                    return _onValueChanged;
                }

                virtual TOnValueChanged DoGetOnDefaultValueChanged() {
                    return _onDefaultValueChanged;
                }

                virtual void DoSet(T value) {
                    _value = value;
                }

                virtual void DoSetDefault(T defaultValue) {
                    _defaultValue = defaultValue;
                }

            public:
                Property(const char* name, T value, T defaultValue, TOnValueChanged onValueChanged = nullptr) : _name(strdup(name)), _value(value), _defaultValue(defaultValue), _onValueChanged(onValueChanged) { }

                Property(const char* name, T value, TOnValueChanged onValueChanged = nullptr) : _name(strdup(name)), _value(value), _defaultValue(value), _onValueChanged(onValueChanged) { }

                virtual ~Property() { 
                    free((void*)_name);
                }

                // Default operators for get and set
                operator T() const {
                    return DoGet();
                }

                T operator=(T value) {
                    SetValue(value);
                    return value;
                }

                // Methods

                void WriteToJson(JsonArray& array) {
                    this->AddPropertyValue(_name, array, DoGet());
                }

                void WriteToJson(JsonObject& object) {
                    object[_name] = DoGet();
                }

                void WriteToJson(JsonDocument& document) {
                    document[_name] = DoGet();
                }

                void WriteDefaultToJson(JsonArray& array) {
                    AddPropertyValue(_name, array, DoGetDefault());
                }

                void WriteDefaultToJson(JsonObject& object) {
                    object[_name] = DoGetDefault();
                }

                void WriteDefaultToJson(JsonDocument& document) {
                    document[_name] = DoGetDefault();
                }

                void ReadValueFromJson(JsonObject& jsonObject, bool useDefaultValue = false) {
                    if (jsonObject.containsKey(_name)) {
                        SetValue(jsonObject[_name]);
                    }
                    else if (useDefaultValue) {
                        SetValue(DoGetDefault());
                    }
                }
            
                void ReadValueFromJson(JsonDocument& jsonDocument, bool useDefaultValue = false) {
                    if (jsonDocument.containsKey(_name)) {
                        SetValue(jsonDocument[_name]);
                    }
                    else if (useDefaultValue) {
                        SetValue(DoGetDefault());
                    }
                }

                void ResetToDefault() {
                    SetValue(DoGetDefault());
                }

                // Getters

                const char* GetName() {
                    return _name;
                }

                T GetValue() {
                    return DoGet();
                }

                T GetDefaultValue() {
                    return DoGetDefault();
                }

                TOnValueChanged GetOnValueChanged() {
                    return DoGetOnValueChanged();
                }

                TOnValueChanged GetOnDefaultValueChanged() {
                    return DoGetOnDefaultValueChanged();
                }

                // Setters

                virtual void SetValue(T value) {
                    // Print the value to the Serial port
                    T oldValue = GetValue();
                    if (DoCompareEqual(oldValue, value)) { return; } // If the value hasn't changed, don't notify the Parent
                    DoSet(value);
                    TOnValueChanged onValueChanged = GetOnValueChanged();
                    if (onValueChanged != nullptr) { onValueChanged(oldValue, value); } // Notify the Parent that this Property's value has changed (if a callback was provided)
                }

                void SetDefaultValue(T defaultValue) {
                    T oldValue = GetDefaultValue();
                    if (DoCompareEqual(oldValue, defaultValue)) { return; } // If the value hasn't changed, don't notify the Parent
                    DoSetDefault(defaultValue);
                    TOnValueChanged onDefaultValueChanged = GetOnDefaultValueChanged();
                    if (onDefaultValueChanged != nullptr) { onDefaultValueChanged(oldValue, defaultValue); } // Notify the Parent that this Property's default value has changed (if a callback was provided)
                }

                void SetOnValueChanged(TOnValueChanged onValueChanged) {
                    DoSetOnValueChanged(onValueChanged);
                }

                void SetOnDefaultValueChanged(TOnValueChanged onDefaultValueChanged) {
                    DoSetOnDefaultValueChanged(onDefaultValueChanged);
                }
        };

    }

}