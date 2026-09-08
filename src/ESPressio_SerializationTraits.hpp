#pragma once

#include <type_traits>
#include <utility>

namespace ESPressio::Serializable {

    namespace Detail {

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
        struct HasSerializableProperties : std::false_type {
        };

        template<typename T>
        struct HasSerializableProperties<
            T,
            std::void_t<decltype(T::GetSerializableProperties())>
        > : std::true_type {
        };

    }

    /// <summary>Indicates whether a type exposes the ESPressio serializable-property contract.</summary>
    /// <remarks>References and cv-qualifiers are removed before detecting <c>GetSerializableProperties()</c>.</remarks>
    template<typename T>
    inline constexpr bool IsSerializable =
        Detail::HasSerializableProperties<
            std::remove_cv_t<std::remove_reference_t<T>>
        >::value;

}
