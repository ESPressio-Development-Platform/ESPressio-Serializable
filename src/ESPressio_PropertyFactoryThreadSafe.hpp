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
                virtual void DoRegisterProperty(IProperty* property) {
                    std::unique_lock<std::shared_mutex> lock(_propertiesMutex);
                    PropertyFactory::DoRegisterProperty(property);
                }

                virtual void DoRegisterProperties(std::initializer_list<IProperty*> properties) {
                    std::unique_lock<std::shared_mutex> lock(_propertiesMutex);
                    PropertyFactory::DoRegisterProperties(properties);
                }

                virtual void DoUnregisterProperty(IProperty* property) {
                    std::unique_lock<std::shared_mutex> lock(_propertiesMutex);
                    PropertyFactory::DoUnregisterProperty(property);
                }

                virtual void DoUnregisterProperty(const char* propertyName) {
                    std::unique_lock<std::shared_mutex> lock(_propertiesMutex);
                    PropertyFactory::DoUnregisterProperty(propertyName);
                }

                virtual void DoClearProperties() {
                    std::unique_lock<std::shared_mutex> lock(_propertiesMutex);
                    PropertyFactory::DoClearProperties();
                }

                virtual uint16_t DoWithProperties(std::function<void(IProperty*)> propertyFunction) {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWithProperties(propertyFunction);
                }

                virtual uint16_t DoWritePropertiesToJson(JsonArray& array) {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWritePropertiesToJson(array);
                }

                virtual uint16_t DoWritePropertiesToJson(JsonObject& object) {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWritePropertiesToJson(object);
                }

                virtual uint16_t DoWritePropertiesToJson(JsonDocument& document) {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWritePropertiesToJson(document);
                }

                virtual uint16_t DoWriteDefaultPropertiesToJson(JsonArray& array) {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWriteDefaultPropertiesToJson(array);
                }

                virtual uint16_t DoWriteDefaultPropertiesToJson(JsonObject& object) {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWriteDefaultPropertiesToJson(object);
                }

                virtual uint16_t DoWriteDefaultPropertiesToJson(JsonDocument& document) {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoWriteDefaultPropertiesToJson(document);
                }

                virtual uint16_t DoReadPropertiesFromJson(JsonObject& jsonObject, bool useDefaultValue = false) {
                    std::shared_lock<std::shared_mutex> lock(_propertiesMutex);
                    return PropertyFactory::DoReadPropertiesFromJson(jsonObject, useDefaultValue);
                }

                virtual uint16_t DoReadPropertiesFromJson(JsonDocument& jsonDocument, bool useDefaultValue = false) {
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