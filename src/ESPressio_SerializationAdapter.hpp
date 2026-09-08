#pragma once
#include <type_traits>
namespace ESPressio::Serializable {
    class SerializationNode;

    /// <summary>Customization point for converting a C++ type to and from <c>SerializationNode</c> representations.</summary>
    /// <typeparam name="T">Type supported by the adapter specialization.</typeparam>
    /// <remarks>The unspecialized adapter reports <c>Supported == false</c>; supported types provide a specialization defining the required conversion operations.</remarks>
/**
 * ESPressio Memory Audit
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 0 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
template<typename T, typename = void>
    struct SerializationAdapter {
        /// <summary>Indicates whether this adapter specialization supports the requested type.</summary>
        static constexpr bool Supported = false;
    };

    /// <summary>Indicates whether a serialization adapter is available for the normalized form of a type.</summary>
    template<typename T>
    inline constexpr bool HasSerializationAdapter =
        SerializationAdapter<
            std::remove_cv_t<std::remove_reference_t<T>>
        >::Supported;
}
