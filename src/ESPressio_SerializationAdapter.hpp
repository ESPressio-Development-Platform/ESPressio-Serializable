#pragma once
#include <type_traits>
namespace ESPressio::Serializable {
    class SerializationNode;

    /// <summary>Customization point for converting a C++ type to and from <c>SerializationNode</c> representations.</summary>
    /// <typeparam name="T">Type supported by the adapter specialization.</typeparam>
    /// <remarks>The unspecialized adapter reports <c>Supported == false</c>; supported types provide a specialization defining the required conversion operations.</remarks>
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
