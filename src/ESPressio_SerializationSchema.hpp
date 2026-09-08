#pragma once
#include <cstdint>
#include <type_traits>
#include <utility>
#include "ESPressio_SerializationNode.hpp"

namespace ESPressio::Serializable::Detail {

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(std::false_type) [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: sizeof(std::false_type) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
template<typename T, typename = void>
    struct HasSchemaVersion : std::false_type {};

    template<typename T>
    struct HasSchemaVersion<
        T,
        std::void_t<decltype(T::GetSerializableSchemaVersion())>
    > : std::true_type {};

    /// <summary>Returns a serializable type's declared schema version, defaulting to version 1.</summary>
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

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(std::false_type) [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: sizeof(std::false_type) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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

    /// <summary>Applies sequential schema migrations to a serialization tree until the target version is reached.</summary>
    /// <typeparam name="T">Serializable type providing an optional static <c>Migrate</c> function.</typeparam>
    /// <returns><c>true</c> when the source already matches the target or every required migration succeeds.</returns>
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
