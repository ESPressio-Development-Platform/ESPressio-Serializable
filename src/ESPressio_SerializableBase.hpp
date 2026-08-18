#pragma once
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>
#include "ESPressio_SerializationSchema.hpp"

namespace ESPressio::Serializable::Detail {
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
            ~SerializableBase() = default;

        private:
            template<typename TArchive, typename TProperty>
            static bool ReadProperty(
                TArchive& archive,
                const TProperty& property,
                TDerived& object
            ) {
                if (property.IsReadOnly()) {
                    return true;
                }

                auto& value = property.GetValue(object);

                if constexpr (Detail::HasContains<TArchive>::value) {
                    if (archive.Contains(property.GetName())) {
                        return archive.Read(property.GetName(), value) && property.ValidateValue(value);
                    }
                } else if (archive.Read(property.GetName(), value)) {
                    return property.ValidateValue(value);
                }

                for (size_t index = 0; index < property.GetAliasCount(); ++index) {
                    const char* alias = property.GetAlias(index);
                    if (alias == nullptr) continue;
                    if constexpr (Detail::HasContains<TArchive>::value) {
                        if (archive.Contains(alias)) {
                            return archive.Read(alias, value) && property.ValidateValue(value);
                        }
                    } else if (archive.Read(alias, value)) {
                        return property.ValidateValue(value);
                    }
                }

                if (property.HasDefault()) {
                    value = property.GetDefault();
                    return property.ValidateValue(value);
                }

                return !property.IsRequired();
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
            bool Deserialize(TArchive& archive) {
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
                        return false;
                    }
                } else if (sourceVersion != GetSchemaVersion()) {
                    return false;
                }

                bool success = true;

                std::apply(
                    [&](const auto&... property) {
                        (
                            (success =
                                ReadProperty(
                                    archive,
                                    property,
                                    object
                                ) &&
                                success),
                            ...
                        );
                    },
                    TDerived::GetSerializableProperties()
                );

                return success;
            }
    };

    template<typename TDerived>
    using Serializable = SerializableBase<TDerived>;

}
