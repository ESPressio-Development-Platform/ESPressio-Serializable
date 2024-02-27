#pragma once

#include <functional>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

#include <ArduinoJson.h>

#include "ESPressio_IProperty.hpp"
#include "ESPressio_PropertyFactory.hpp"

namespace ESPressio {

    namespace Serializable {

        class PropertyFactoryThreadSafe : public PropertyFactory {
            private:
                std::shared_mutex _propertiesMutex;
            protected:
                IProperty* DoGetProperty(const char* propertyName) override {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoGetProperty(propertyName);
                }

                void DoRegisterProperty(IProperty* property) override {
                    std::unique_lock<std::shared_mutex> lock(_propertiesMutex);
                    PropertyFactory::DoRegisterProperty(property);
                }

                void DoRegisterProperties(std::initializer_list<IProperty*> properties) override {
                    std::unique_lock<std::shared_mutex> lock(_propertiesMutex);
                    PropertyFactory::DoRegisterProperties(properties);
                }

                void DoUnregisterProperty(IProperty* property) override {
                    std::unique_lock<std::shared_mutex> lock(_propertiesMutex);
                    PropertyFactory::DoUnregisterProperty(property);
                }

                void DoUnregisterProperty(const char* propertyName) override {
                    std::unique_lock<std::shared_mutex> lock(_propertiesMutex);
                    PropertyFactory::DoUnregisterProperty(propertyName);
                }

                void DoClearProperties() override {
                    std::unique_lock<std::shared_mutex> lock(_propertiesMutex);
                    PropertyFactory::DoClearProperties();
                }

                uint16_t DoWithProperties(std::function<void(IProperty*)> propertyFunction) override {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWithProperties(propertyFunction);
                }

                uint16_t DoWritePropertiesToJson(JsonArray& array) override {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWritePropertiesToJson(array);
                }

                uint16_t DoWritePropertiesToJson(JsonObject& object) override {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWritePropertiesToJson(object);
                }

                uint16_t DoWritePropertiesToJson(JsonDocument& document) override {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWritePropertiesToJson(document);
                }

                uint16_t DoWriteDefaultPropertiesToJson(JsonArray& array) override {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWriteDefaultPropertiesToJson(array);
                }

                uint16_t DoWriteDefaultPropertiesToJson(JsonObject& object) override {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWriteDefaultPropertiesToJson(object);
                }

                uint16_t DoWriteDefaultPropertiesToJson(JsonDocument& document) override {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWriteDefaultPropertiesToJson(document);
                }

                uint16_t DoReadPropertiesFromJson(JsonObject& jsonObject, bool useDefaultValue = false) override {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoReadPropertiesFromJson(jsonObject, useDefaultValue);
                }

                uint16_t DoReadPropertiesFromJson(JsonDocument& jsonDocument, bool useDefaultValue = false) override {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoReadPropertiesFromJson(jsonDocument, useDefaultValue);
                }
            public:
                PropertyFactoryThreadSafe() {

                }

                ~PropertyFactoryThreadSafe() {
                    std::unique_lock<std::shared_mutex> lock(_propertiesMutex);
                }
        };

    }

}