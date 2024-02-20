#pragma once

#include <functional>

#include <ArduinoJson.h>

#include "ESPressio_IProperty.hpp"


namespace ESPressio {

    namespace Serializable {

        template <typename T>
        class Property : public IProperty {
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

                virtual void DoSet(T value) {
                    _value = value;
                }

                virtual void DoSetDefault(T defaultValue) {
                    _defaultValue = defaultValue;
                }

            public:
                Property(const char* name, T value, T defaultValue, TOnValueChanged onValueChanged = nullptr) : IProperty(name), _value(value), _defaultValue(defaultValue), _onValueChanged(onValueChanged) { }

                Property(const char* name, T value, TOnValueChanged onValueChanged = nullptr) : IProperty(name), _value(value), _defaultValue(value), _onValueChanged(onValueChanged) { }

                // Default operators for get and set
                operator T() const {
                    return DoGet();
                }

                T operator=(T value) {
                    SetValue(value);
                    return _value;
                }

                // Methods

                void WriteToJson(JsonArray& array) {
                    this->AddPropertyValue(_name, array, _value);
                }

                void WriteToJson(JsonObject& object) {
                    object[_name] = _value;
                }

                void WriteToJson(JsonDocument& document) {
                    document[_name] = _value;
                }

                void WriteDefaultToJson(JsonArray& array) {
                    AddPropertyValue(_name, array, _defaultValue);
                }

                void WriteDefaultToJson(JsonObject& object) {
                    object[_name] = _defaultValue;
                }

                void WriteDefaultToJson(JsonDocument& document) {
                    document[_name] = _defaultValue;
                }

                void ReadValueFromJson(JsonObject& jsonObject, bool useDefaultValue = false) {
                    if (jsonObject.containsKey(_name)) {
                        SetValue(jsonObject[_name]);
                    }
                    else if (useDefaultValue) {
                        SetValue(_defaultValue);
                    }
                }
            
                void ReadValueFromJson(JsonDocument& jsonDocument, bool useDefaultValue = false) {
                    if (jsonDocument.containsKey(_name)) {
                        SetValue(jsonDocument[_name]);
                    }
                    else if (useDefaultValue) {
                        SetValue(_defaultValue);
                    }
                }

                void ResetToDefault() {
                    SetValue(_defaultValue);
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
                    return _onValueChanged;
                }

                TOnValueChanged GetOnDefaultValueChanged() {
                    return _onDefaultValueChanged;
                }

                // Setters

                virtual void SetValue(T value) {
                    // Print the value to the Serial port
                    T oldValue = GetValue();
                    if (DoCompareEqual(oldValue, _value)) { return; } // If the value hasn't changed, don't notify the Parent
                    DoSet(value);
                    if (_onValueChanged != nullptr) { _onValueChanged(oldValue, _value); } // Notify the Parent that this Property's value has changed (if a callback was provided)
                }

                void SetDefaultValue(T defaultValue) {
                    T oldValue = GetDefaultValue();
                    if (DoCompareEqual(oldValue, _defaultValue)) { return; } // If the value hasn't changed, don't notify the Parent
                    DoSetDefault(defaultValue);
                    if (_onDefaultValueChanged != nullptr) { _onDefaultValueChanged(oldValue, _defaultValue); } // Notify the Parent that this Property's default value has changed (if a callback was provided)
                }

                void SetOnValueChanged(TOnValueChanged onValueChanged) {
                    _onValueChanged = onValueChanged;
                }

                void SetOnDefaultValueChanged(TOnValueChanged onDefaultValueChanged) {
                    _onDefaultValueChanged = onDefaultValueChanged;
                }
        };

    }

}