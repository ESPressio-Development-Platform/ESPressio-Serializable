#pragma once
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>
#include "ESPressio_SerializationSchema.hpp"
#include "ESPressio_SerializationResult.hpp"

namespace ESPressio::Serializable::Detail {
    template<typename TArchive, typename TValue, typename = void>
    struct HasReadDetailed : std::false_type {};

    template<typename TArchive, typename TValue>
    struct HasReadDetailed<TArchive,TValue,std::void_t<decltype(std::declval<TArchive&>().ReadDetailed(std::declval<const char*>(),std::declval<TValue&>(),std::declval<const DeserializationOptions&>()))>> : std::true_type {};

    template<typename TArchive, typename = void>
    struct HasGetNode : std::false_type {};

    template<typename TArchive, typename = void>
    struct HasContains : std::false_type {};

    template<typename TArchive>
    struct HasContains<TArchive,std::void_t<decltype(std::declval<const TArchive&>().Contains(std::declval<const char*>()))>> : std::true_type {};

    template<typename TArchive, typename TProperty, typename TValue, typename = void>
    struct HasWriteProperty : std::false_type {};

    template<typename TArchive, typename TProperty, typename TValue>
    struct HasWriteProperty<TArchive,TProperty,TValue,std::void_t<decltype(std::declval<TArchive&>().WriteProperty(std::declval<const TProperty&>(),std::declval<const TValue&>()))>> : std::true_type {};

    template<typename TArchive>
    struct HasGetNode<
        TArchive,
        std::void_t<decltype(std::declval<TArchive&>().GetNode())>
    > : std::true_type {};
}

namespace ESPressio::Serializable {

    template<typename TDerived>
    class SerializableBase {
        protected:
            constexpr SerializableBase() = default;

        public:
            ~SerializableBase() = default;

        private:
            template<typename TArchive, typename TProperty>
            static bool ReadPropertyDetailed(
                TArchive& archive,
                const TProperty& property,
                TDerived& object,
                DeserializationResult& result,
                const DeserializationOptions& options
            ) {
                if (property.IsReadOnly()) return true;
                auto& value = property.GetValue(object);
                const char* canonical = property.GetName();

                auto tryName = [&](const char* name) -> int {
                    if (name == nullptr) return 0;
                    if constexpr (Detail::HasContains<TArchive>::value) {
                        if (!archive.Contains(name)) return 0;
                    }
                    if constexpr (Detail::HasReadDetailed<TArchive,decltype(value)>::value) {
                        auto nested = archive.ReadDetailed(name, value, options);
                        if (!nested) { result.Merge(nested, "", options); return -1; }
                    } else {
                        if (!archive.Read(name, value)) {
                            result.Add(SerializationErrorCode::TypeMismatch, name, "Property is present but could not be converted to the declared C++ type", options);
                            return -1;
                        }
                    }
                    if (!property.ValidateValue(value)) {
                        result.Add(SerializationErrorCode::ValidationFailed, canonical ? canonical : name, "Property value failed its validator or numeric range constraint", options);
                        return -1;
                    }
                    return 1;
                };

                int status = tryName(canonical);
                if (status != 0) return status > 0;

                for (size_t index = 0; index < property.GetAliasCount(); ++index) {
                    status = tryName(property.GetAlias(index));
                    if (status != 0) return status > 0;
                }

                if (property.HasDefault()) {
                    value = property.GetDefault();
                    if (!property.ValidateValue(value)) {
                        result.Add(SerializationErrorCode::ValidationFailed, canonical ? canonical : "", "Configured default value failed property validation", options);
                        return false;
                    }
                    return true;
                }

                if (property.IsRequired()) {
                    result.Add(SerializationErrorCode::MissingRequiredProperty, canonical ? canonical : "", "Required property is absent (including all aliases)", options);
                    return false;
                }
                return true;
            }

        public:
            static constexpr uint32_t GetSchemaVersion() {
                return Detail::SchemaVersion<TDerived>();
            }

            template<typename TArchive>
            void Serialize(TArchive& archive) const {
                const TDerived& object =
                    static_cast<const TDerived&>(*this);

                archive.Write("__schemaVersion", GetSchemaVersion());

                std::apply(
                    [&](const auto&... property) {
                        (
                            ([&]() {
                                const auto& value = property.GetValue(object);
                                using PropertyType = std::decay_t<decltype(property)>;
                                using ValueType = std::decay_t<decltype(value)>;
                                if constexpr (Detail::HasWriteProperty<TArchive,PropertyType,ValueType>::value) {
                                    archive.WriteProperty(property,value);
                                } else {
                                    archive.Write(property.GetName(),value);
                                }
                            }()),
                            ...
                        );
                    },
                    TDerived::GetSerializableProperties()
                );
            }

            template<typename TArchive>
            DeserializationResult DeserializeDetailed(
                TArchive& archive,
                const DeserializationOptions& options = {}
            ) {
                DeserializationResult result;
                TDerived& object =
                    static_cast<TDerived&>(*this);

                uint32_t sourceVersion = 1u;
                archive.Read("__schemaVersion", sourceVersion);

                if constexpr (Detail::HasGetNode<TArchive>::value) {
                    if (
                        !Detail::ApplyMigrations<TDerived>(
                            archive.GetNode(),
                            sourceVersion,
                            GetSchemaVersion()
                        )
                    ) {
                        result.Add(SerializationErrorCode::MigrationFailed,"__schemaVersion","Schema migration failed", options); return result;
                    }
                } else if (sourceVersion != GetSchemaVersion()) {
                    result.Add(SerializationErrorCode::UnsupportedSchemaVersion,"__schemaVersion","Archive does not support structural migration", options); return result;
                }

                bool success = true;

                std::apply(
                    [&](const auto&... property) {
                        (
                            (success = result.ShouldContinue(options) &&
                                ReadPropertyDetailed(
                                    archive,
                                    property,
                                    object,
                                    result,
                                    options
                                ) &&
                                success),
                            ...
                        );
                    },
                    TDerived::GetSerializableProperties()
                );

                (void)success;
                return result;
            }
            template<typename TArchive>
            bool Deserialize(TArchive& archive) { return DeserializeDetailed(archive).Success(); }

    };

    template<typename TDerived>
    using Serializable = SerializableBase<TDerived>;

}
