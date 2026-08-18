#pragma once
#include <cstdint>
#include <type_traits>
#include <utility>
#include "ESPressio_SerializationNode.hpp"

namespace ESPressio::Serializable::Detail {

    template<typename T, typename = void>
    struct HasSchemaVersion : std::false_type {};

    template<typename T>
    struct HasSchemaVersion<
        T,
        std::void_t<decltype(T::GetSerializableSchemaVersion())>
    > : std::true_type {};

    template<typename T>
    constexpr uint32_t SchemaVersion() {
        if constexpr (HasSchemaVersion<T>::value) {
            return static_cast<uint32_t>(
                T::GetSerializableSchemaVersion()
            );
        } else {
            return 1u;
        }
    }

    template<typename T, typename = void>
    struct HasMigration : std::false_type {};

    template<typename T>
    struct HasMigration<
        T,
        std::void_t<
            decltype(
                T::Migrate(
                    std::declval<SerializationNode&>(),
                    std::declval<uint32_t>(),
                    std::declval<uint32_t>()
                )
            )
        >
    > : std::true_type {};

    template<typename T>
    bool ApplyMigrations(
        SerializationNode& node,
        uint32_t fromVersion,
        uint32_t targetVersion
    ) {
        if (fromVersion == 0u) {
            fromVersion = 1u;
        }

        if (fromVersion > targetVersion) {
            return false;
        }

        if (fromVersion == targetVersion) {
            return true;
        }

        if constexpr (!HasMigration<T>::value) {
            return false;
        } else {
            for (
                uint32_t current = fromVersion;
                current < targetVersion;
                ++current
            ) {
                if (!T::Migrate(node, current, current + 1u)) {
                    return false;
                }
            }

            return true;
        }
    }

}
